#include "AI/7WDuel/GameEngine.h"
#include "AI/blockusAI/BlokusAI.h"
#include "Core/Algorithms.h"

#include <sstream>

namespace sevenWD
{
	namespace Helper
	{
		bool isReplayWonder(Wonders _wonder)
		{
			switch (_wonder)
			{
			case Wonders::HangingGarden:
			case Wonders::Atremis:
			case Wonders::Sphinx:
			case Wonders::ViaAppia:
			case Wonders::Piraeus:
				return true;
			}
			return false;
		}

		const char * toString(ResourceType _type)
		{
			switch (_type)
			{
			case ResourceType::Wood:	return "Wood";
			case ResourceType::Clay:	return "Clay";
			case ResourceType::Stone:	return "Stone";
			case ResourceType::Glass:	return "Glass";
			case ResourceType::Papyrus: return "Papyrus";
			default:
				return "None";
			}
			
			
		};

		const char* toString(CardType _type)
		{
			switch (_type)
			{
			case CardType::Blue:	return "Blue";
			case CardType::Brown:	return "Brown";
			case CardType::Grey:	return "Grey";
			case CardType::Yellow:	return "Yellow";
			case CardType::Science:	return "Science";
			case CardType::Military:return "Military";
			case CardType::Guild:	return "Guild";
			case CardType::ScienceToken: return "Token";
			case CardType::Wonder:	return "Wonder";

			default:
				return "None";
			}
		}

		const char* toString(ScienceSymbol _type)
		{
			switch (_type)
			{
			case ScienceSymbol::Wheel:		return "Wheel";
			case ScienceSymbol::Script:		return "Script";
			case ScienceSymbol::Triangle:	return "Triangle";
			case ScienceSymbol::Bowl:		return "Bowl";
			case ScienceSymbol::SolarClock:	return "SolarClock";
			case ScienceSymbol::Globe:		return "Globe";
			case ScienceSymbol::Law:		return "Law";

			default:
				return "None";
			}
		}
	}

	//----------------------------------------------------------------------------
	GameState::GameState() : m_context{ nullptr }, m_playerCity{ nullptr, nullptr }
	{
	}

	//----------------------------------------------------------------------------
	GameState::GameState(const GameContext& _context) : m_context{ &_context }, m_playerCity{ &_context, &_context }
	{
		initScienceTokens();
		m_context->initCityWithRandomWonders(m_playerCity[0], m_playerCity[1]);

		initAge1Graph();
	}

	//----------------------------------------------------------------------------
	const Card& GameState::getPlayableCard(u32 _index) const
	{
		DEBUG_ASSERT(_index < m_numPlayableCards);
		u8 pickedCard = m_playableCards[_index];
		return m_context->getCard(m_graph[pickedCard].m_cardId);
	}

	//----------------------------------------------------------------------------
	const Card& GameState::getPlayableScienceToken(u32 _index) const
	{
		DEBUG_ASSERT(_index < m_numScienceToken);
		return m_context->getScienceToken(m_scienceTokens[_index]);
	}

	//----------------------------------------------------------------------------
	const Card& GameState::getCurrentPlayerWonder(u32 _index) const
	{
		DEBUG_ASSERT(_index < getCurrentPlayerCity().m_unbuildWonderCount);
		Wonders wonder = getCurrentPlayerCity().m_unbuildWonders[_index];
		return m_context->getWonder(wonder);
	}

	SpecialAction GameState::pick(u32 _playableCardIndex)
	{
		DEBUG_ASSERT(_playableCardIndex < m_numPlayableCards);
		u8 pickedCard = m_playableCards[_playableCardIndex];
		std::swap(m_playableCards[_playableCardIndex], m_playableCards[m_numPlayableCards - 1]);
		m_numPlayableCards--;

		unlinkNodeFromGraph(pickedCard);

		const Card& card = m_context->getCard(m_graph[pickedCard].m_cardId);
		if (card.getMilitary() != 0)
		{ 
			u8 militaryBonus = getCurrentPlayerCity().ownScienceToken(ScienceToken::Strategy);
			m_military += (m_playerTurn == 0 ? (card.getMilitary() + militaryBonus) : -(card.getMilitary() + militaryBonus));

			if (m_military >= 3 && !militaryToken2[0])
			{
				militaryToken2[0] = true;
				m_playerCity[1].m_gold = Helper::safeSub<u8>(m_playerCity[1].m_gold, 2);
			}
			if (m_military >= 6 && !militaryToken5[0])
			{
				militaryToken5[0] = true;
				m_playerCity[1].m_gold = Helper::safeSub<u8>(m_playerCity[1].m_gold, 5);
			}
			if (m_military <= -3 && !militaryToken2[1])
			{
				militaryToken2[1] = true;
				m_playerCity[0].m_gold = Helper::safeSub<u8>(m_playerCity[0].m_gold, 2);
			}
			if (m_military <= -6 && !militaryToken5[1])
			{
				militaryToken5[1] = true;
				m_playerCity[0].m_gold = Helper::safeSub<u8>(m_playerCity[0].m_gold, 5);
			}
		}

		auto& otherPlayer = m_playerCity[(m_playerTurn + 1) % 2];
		u32 cost = getCurrentPlayerCity().computeCost(card, otherPlayer);

		DEBUG_ASSERT(getCurrentPlayerCity().m_gold >= cost);
		getCurrentPlayerCity().m_gold -= u8(cost);

		if (otherPlayer.ownScienceToken(ScienceToken::Economy) && cost >= card.getGoldCost())
		{
			otherPlayer.m_gold += (u8(cost) - card.getGoldCost());
		}

		SpecialAction action = getCurrentPlayerCity().addCard(card, otherPlayer);

		if (abs(m_military) >= 9)
			return SpecialAction::MilitaryWin;
		else 
			return action;
	}

	void GameState::burn(u32 _playableCardIndex)
	{
		DEBUG_ASSERT(_playableCardIndex < m_numPlayableCards);
		u8 pickedCard = m_playableCards[_playableCardIndex];
		std::swap(m_playableCards[_playableCardIndex], m_playableCards[m_numPlayableCards - 1]);
		m_numPlayableCards--;

		unlinkNodeFromGraph(pickedCard);

		u8 burnValue = 2 + getCurrentPlayerCity().m_numCardPerType[u32(CardType::Yellow)];
		getCurrentPlayerCity().m_gold += burnValue;
	}

	SpecialAction GameState::buildWonder(u32 _withPlayableCardIndex, u32 _wondersIndex, u8 _additionalEffect)
	{
		DEBUG_ASSERT(_withPlayableCardIndex < m_numPlayableCards);
		u8 pickedCard = m_playableCards[_withPlayableCardIndex];
		std::swap(m_playableCards[_withPlayableCardIndex], m_playableCards[m_numPlayableCards - 1]);
		m_numPlayableCards--;

		unlinkNodeFromGraph(pickedCard);

		Wonders pickedWonder = getCurrentPlayerCity().m_unbuildWonders[_wondersIndex];
		std::swap(getCurrentPlayerCity().m_unbuildWonders[_wondersIndex], getCurrentPlayerCity().m_unbuildWonders[getCurrentPlayerCity().m_unbuildWonderCount - 1]);
		getCurrentPlayerCity().m_unbuildWonderCount--;

		const Card& wonder = m_context->getWonder(pickedWonder);
		auto& otherPlayer = m_playerCity[(m_playerTurn + 1) % 2];
		u32 cost = getCurrentPlayerCity().computeCost(wonder, otherPlayer);

		DEBUG_ASSERT(getCurrentPlayerCity().m_gold >= cost);
		getCurrentPlayerCity().m_gold -= u8(cost);

		if (pickedWonder == Wonders::ViaAppia)
			otherPlayer.m_gold = Helper::safeSub<u8>(otherPlayer.m_gold, 3);

		else if (_additionalEffect != u8(-1) && (pickedWonder == Wonders::Zeus || pickedWonder == Wonders::CircusMaximus))
			otherPlayer.removeCard(m_context->getCard(_additionalEffect));

		else if (_additionalEffect != u8(-1) && pickedWonder == Wonders::Mausoleum)
		{
			const Card& card = m_context->getCard(_additionalEffect);
			getCurrentPlayerCity().addCard(card, otherPlayer);
		}

		SpecialAction action =  getCurrentPlayerCity().addCard(wonder, otherPlayer);
		if (abs(m_military) >= 9)
			return SpecialAction::MilitaryWin;
		else
			return action;
	}

	SpecialAction GameState::pickScienceToken(u32 _tokenIndex)
	{
		ScienceToken pickedToken = m_scienceTokens[_tokenIndex];
		std::swap(m_scienceTokens[_tokenIndex], m_scienceTokens[m_numScienceToken - 1]);
		m_numScienceToken--;

		return getCurrentPlayerCity().addCard(m_context->getScienceToken(pickedToken), getOtherPlayerCity());
	}

	GameState::NextAge GameState::nextAge()
	{
		if (m_numPlayableCards == 0)
		{
			if (m_currentAge == 0)
				initAge2Graph();
			else if (m_currentAge == 1)
				initAge3Graph();
			else if (m_currentAge == 2)
				return NextAge::EndGame;

			if (m_military < 0) // player 1 is advanced in military, player 0 to play
				m_playerTurn = 0;
			else if (m_military > 0)
				m_playerTurn = 1;
			else
				; // nothing to do, last player is the player to start the turn

			return NextAge::Next;
		}
		return NextAge::None;
	}

	u32 GameState::findWinner()
	{
		u32 vp0 = m_playerCity[0].computeVictoryPoint(m_playerCity[1]);
		u32 vp1 = m_playerCity[1].computeVictoryPoint(m_playerCity[0]);
		if (m_military >= 6)
			vp0 += 10;
		else if (m_military >= 3)
			vp0 += 5;
		else if (m_military >= 1)
			vp0 += 2;

		if (m_military <= -6)
			vp1 += 10;
		else if (m_military <= -3)
			vp1 += 5;
		else if (m_military <= -1)
			vp1 += 2;

		if (vp0 == vp1)
			return m_playerCity[0].m_numCardPerType[u32(CardType::Blue)] > m_playerCity[1].m_numCardPerType[u32(CardType::Blue)] ? 0 : 1;
		else
			return vp0 < vp1 ? 1 : 0;
	}

	u32 GameState::genPyramidGraph(u32 _numRow, u32 _startNodeIndex)
	{
		u32 prevRowStartIndex = u32(-1);
		u32 curNodeIndex = _startNodeIndex;

		for (u32 row = 0; row < _numRow; ++row)
		{
			for (u32 i = 0; i < 2 + row; ++i)
			{
				m_graph[curNodeIndex + i].m_isGuildCard = 0;
				m_graph[curNodeIndex + i].m_visible = row % 2 == 0;
				m_graph[curNodeIndex + i].m_child0 = CardNode::InvalidNode;
				m_graph[curNodeIndex + i].m_child1 = CardNode::InvalidNode;

				if (prevRowStartIndex != u32(-1)) // not first row
				{
					if (i == 0) // first card on the row
					{
						m_graph[curNodeIndex + i].m_parent0 = prevRowStartIndex;
						m_graph[prevRowStartIndex].m_child0 = curNodeIndex + i;

						m_graph[curNodeIndex + i].m_parent1 = CardNode::InvalidNode;
					}
					else if (i == 1 + row) // last card on the row
					{
						m_graph[curNodeIndex + i].m_parent0 = prevRowStartIndex + row;
						m_graph[prevRowStartIndex + row].m_child1 = curNodeIndex + i;

						m_graph[curNodeIndex + i].m_parent1 = CardNode::InvalidNode;
					}
					else
					{
						m_graph[curNodeIndex + i].m_parent0 = prevRowStartIndex + i - 1;
						m_graph[curNodeIndex + i].m_parent1 = prevRowStartIndex + i;

						m_graph[prevRowStartIndex + i - 1].m_child1 = curNodeIndex + i;
						m_graph[prevRowStartIndex + i].m_child0 = curNodeIndex + i;
					}
				}
				else
				{
					m_graph[curNodeIndex + i].m_parent0 = CardNode::InvalidNode;
					m_graph[curNodeIndex + i].m_parent1 = CardNode::InvalidNode;
				}
			}

			prevRowStartIndex = curNodeIndex;
			curNodeIndex += 2 + row;
		}

		return curNodeIndex;
	}

	u32 GameState::genInversePyramidGraph(u32 _baseSize, u32 _numRow, u32 _startNodeIndex)
	{
		u32 prevRowStartIndex = u32(-1);
		u32 curNodeIndex = _startNodeIndex;

		for (u32 row = 0; row < _numRow; ++row)
		{
			for (u32 i = 0; i < _baseSize - row; ++i)
			{
				m_graph[curNodeIndex + i].m_isGuildCard = 0;
				m_graph[curNodeIndex + i].m_visible = row % 2 == 0;
				m_graph[curNodeIndex + i].m_child0 = CardNode::InvalidNode;
				m_graph[curNodeIndex + i].m_child1 = CardNode::InvalidNode;

				if (prevRowStartIndex != u32(-1))
				{
					m_graph[curNodeIndex + i].m_parent0 = prevRowStartIndex + i;
					m_graph[curNodeIndex + i].m_parent1 = prevRowStartIndex + i + 1;

					m_graph[prevRowStartIndex + i].m_child1 = curNodeIndex + i;
					m_graph[prevRowStartIndex + 1 + i].m_child0 = curNodeIndex + i;
				}
				else
				{
					m_graph[curNodeIndex + i].m_parent0 = CardNode::InvalidNode;
					m_graph[curNodeIndex + i].m_parent1 = CardNode::InvalidNode;
				}
			}

			prevRowStartIndex = curNodeIndex;
			curNodeIndex += _baseSize - row;
		}

		return curNodeIndex;
	}

	void GameState::unlinkNodeFromGraph(u32 _nodeIndex)
	{
		DEBUG_ASSERT(m_graph[_nodeIndex].m_child0 == CardNode::InvalidNode && m_graph[_nodeIndex].m_child1 == CardNode::InvalidNode);

		auto removeFromParent = [&](u8 parent)
		{
			if (parent != CardNode::InvalidNode)
			{
				m_graph[parent].m_child0 = m_graph[parent].m_child0 == _nodeIndex ? CardNode::InvalidNode : m_graph[parent].m_child0;
				m_graph[parent].m_child1 = m_graph[parent].m_child1 == _nodeIndex ? CardNode::InvalidNode : m_graph[parent].m_child1;

				if (m_graph[parent].m_child0 == CardNode::InvalidNode && m_graph[parent].m_child1 == CardNode::InvalidNode)
				{
					if(m_graph[parent].m_visible == 0)
						pickCardAdnInitNode(m_graph[parent]);
					m_playableCards[m_numPlayableCards++] = parent;
				}
			}
		};
		removeFromParent(m_graph[_nodeIndex].m_parent0);
		removeFromParent(m_graph[_nodeIndex].m_parent1);
	}

	void GameState::pickCardAdnInitNode(CardNode& _node)
	{
		_node.m_visible = 1;
		if (_node.m_isGuildCard)
		{
			u8 index = pickCardIndex(m_availableGuildCards, m_numAvailableGuildCards);
			_node.m_cardId = m_context->getGuildCard(index).getId();
		}
		else
		{
			u8 index = pickCardIndex(m_availableAgeCards, m_numAvailableAgeCards);
			switch (m_currentAge)
			{
			default:
				DEBUG_ASSERT(0); break;
			case 0:
				_node.m_cardId = m_context->getAge1Card(index).getId(); break;
			case 1:
				_node.m_cardId = m_context->getAge2Card(index).getId(); break;
			case 2:
				_node.m_cardId = m_context->getAge3Card(index).getId(); break;
			};
		}
	}

	void GameState::initScienceTokens()
	{
		m_scienceTokens = {
			ScienceToken::Agriculture,
			ScienceToken::TownPlanning,
			ScienceToken::Architecture,
			ScienceToken::Theology,
			ScienceToken::Strategy,
			ScienceToken::Law,
			ScienceToken::Mathematics,
			ScienceToken::Masonry,
			ScienceToken::Philosophy,
			ScienceToken::Economy 
		};

		std::shuffle(std::begin(m_scienceTokens), std::end(m_scienceTokens), m_context->rand());
		m_numScienceToken = 5; // 5 first tokens are used on the board
	}

	void GameState::initAge1Graph()
	{
		m_currentAge = 0;
		u32 end = genPyramidGraph(5, 0);

		m_numPlayableCards = 0;
		for (u32 i = 0; i < 6; ++i)
			m_playableCards[m_numPlayableCards++] = u8(end - 6 + i);

		m_numAvailableAgeCards = m_context->getAge1CardCount();
		for (u32 i = 0; i < m_numAvailableAgeCards; ++i)
			m_availableAgeCards[i] = u8(i);

		for (CardNode& node : m_graph)
		{
			if (node.m_visible)
				pickCardAdnInitNode(node);
		}
	}

	void GameState::initAge2Graph()
	{
		m_currentAge = 1;
		u32 end = genInversePyramidGraph(6, 5, 0);

		m_numPlayableCards = 0;
		for (u32 i = 0; i < 2; ++i)
			m_playableCards[m_numPlayableCards++] = u8(end - 2 + i);

		m_numAvailableAgeCards = m_context->getAge2CardCount();
		for (u32 i = 0; i < m_numAvailableAgeCards; ++i)
			m_availableAgeCards[i] = u8(i);
		
		for (CardNode& node : m_graph)
		{
			if (node.m_visible)
				pickCardAdnInitNode(node);
		}
	}

	void GameState::initAge3Graph()
	{
		m_currentAge = 2;
		u32 end = genPyramidGraph(3, 0);

		const u32 connectNode0 = end;
		const u32 connectNode1 = end + 1;

		m_graph[connectNode0].m_visible = 0;
		m_graph[connectNode1].m_visible = 0;
		m_graph[connectNode0].m_isGuildCard = 0;
		m_graph[connectNode1].m_isGuildCard = 0;

		m_graph[connectNode0].m_parent0 = 5;
		m_graph[connectNode0].m_parent1 = 6;
		m_graph[5].m_child1 = connectNode0;
		m_graph[6].m_child0 = connectNode0;
		
		m_graph[connectNode1].m_parent0 = 7;
		m_graph[connectNode1].m_parent1 = 8;
		m_graph[7].m_child1 = connectNode1;
		m_graph[8].m_child0 = connectNode1;

		end = genInversePyramidGraph(4, 3, end + 2);

		m_graph[connectNode0].m_child0 = 11;
		m_graph[connectNode0].m_child1 = 12;
		m_graph[11].m_parent1 = connectNode0;
		m_graph[12].m_parent0 = connectNode0;

		m_graph[connectNode1].m_child0 = 13;
		m_graph[connectNode1].m_child1 = 14;
		m_graph[13].m_parent1 = connectNode1;
		m_graph[14].m_parent0 = connectNode1;

		// assign randomely guild cards tag
		std::vector<u8> guildCarTag(m_graph.size(), 0);
		guildCarTag[0] = 1;
		guildCarTag[1] = 1;
		guildCarTag[2] = 1;
		std::shuffle(guildCarTag.begin(), guildCarTag.end(), m_context->rand());
		for (u32 i = 0; i < m_graph.size(); ++i)
			m_graph[i].m_isGuildCard = guildCarTag[i];

		m_numPlayableCards = 0;
		for (u32 i = 0; i < 2; ++i)
			m_playableCards[m_numPlayableCards++] = u8(end - 2 + i);

		m_numAvailableAgeCards = m_context->getAge3CardCount();
		m_numAvailableGuildCards = m_context->getGuildCardCount();

		for (u32 i = 0; i < m_numAvailableAgeCards; ++i)
			m_availableAgeCards[i] = u8(i);
		for (u32 i = 0; i < m_numAvailableGuildCards; ++i)
			m_availableGuildCards[i] = u8(i);
		
		for (CardNode& node : m_graph)
		{
			if (node.m_visible)
				pickCardAdnInitNode(node);
		}
	}

	std::array<ScienceToken, 5> GameState::getUnusedScienceToken() const
	{
		std::array<ScienceToken, 5> tokens;
		for (u32 i = 5; i < u32(ScienceToken::Count); ++i)
			tokens[i - 5] = m_scienceTokens[i];

		return tokens;
	}

	void GameState::printPlayablCards() const
	{
		std::cout << "Player turn : " << u32(m_playerTurn) << std::endl;
		for (u32 i = 0; i < m_numPlayableCards; ++i)
		{
			const Card& card = m_context->getCard(m_graph[m_playableCards[i]].m_cardId);
			std::cout << i+1 << ", Cost= "<< getCurrentPlayerCity().computeCost(card, getOtherPlayerCity()) << " :";
			card.print();
		}
	}

	void GameState::printAvailableTokens() const
	{
		for (u32 i = 0; i < m_numScienceToken; ++i)
		{
			const Card& card = m_context->getScienceToken(m_scienceTokens[i]);
			std::cout << i+1 << ": ";
			card.print();
		}
	}

	void GameState::printGameState() const
	{
		std::cout << "Military = " << int(m_military) << ", Science Token = { ";
		for (u32 i = 0; i < m_numScienceToken; ++i)
			std::cout << m_context->getScienceToken(m_scienceTokens[i]).getName() << " ";
		
		std::cout << "}\n";

		auto printCity = [&](const PlayerCity& _city)
		{
			std::cout << "Gold=" << u32(_city.m_gold) << ", VP=" << u32(_city.m_victoryPoints) << ", Prod={";
			for (u32 i = 0; i < u32(ResourceType::Count); ++i)
				std::cout << u32(_city.m_production[i]) << " ";
			std::cout << "}, Discount={";
			for (u32 i = 0; i < u32(ResourceType::Count); ++i)
				std::cout << u32(_city.m_resourceDiscount[i]) << " ";
			std::cout << " ScienceToken:" << std::bitset<8>(_city.m_ownedScienceTokens) << " ";
			std::cout << "}\n";
		};

		printCity(m_playerCity[0]);
		printCity(m_playerCity[1]);
	}

	u32 PlayerCity::computeCost(const Card& _card, const PlayerCity& _otherPlayer) const
	{
		if (_card.m_chainIn != ChainingSymbol::None && m_chainingSymbols & (1u << u32(_card.m_chainIn)))
			return 0;

		std::array<u8, u32(RT::Count)> goldCostPerResource = { 2,2,2,2,2 }; // base cost
		for (u32 i = 0; i < u32(RT::Count); ++i)
		{
			goldCostPerResource[i] += _otherPlayer.m_production[i];
			goldCostPerResource[i] = m_resourceDiscount[i] ? 1 : goldCostPerResource[i];
		}

		std::array<u8, u32(RT::Count)> cardResourceCost = _card.m_cost;
		bool empty = true;
		for (u32 i = 0; i < u32(RT::Count); ++i)
		{
			cardResourceCost[i] = Helper::safeSub(cardResourceCost[i], m_production[i]);
			empty &= cardResourceCost[i] == 0;
		}
		
		if (!empty)
		{
			ResourceType normalResources[] = { RT::Wood, RT::Clay, RT::Stone };
			ResourceType rareResources[] = { RT::Glass, RT::Papyrus };
			std::sort(std::begin(normalResources), std::end(normalResources), [&](ResourceType _1, ResourceType _2) { return goldCostPerResource[u32(_1)] > goldCostPerResource[u32(_2)]; });
			std::sort(std::begin(rareResources), std::end(rareResources), [&](ResourceType _1, ResourceType _2) { return goldCostPerResource[u32(_1)] > goldCostPerResource[u32(_2)]; });
			

			if (ownScienceToken(ScienceToken::Masonry) && _card.m_type == CardType::Blue ||
				ownScienceToken(ScienceToken::Architecture) && _card.m_type == CardType::Wonder)
			{
				ResourceType allResources[] = { RT::Wood, RT::Clay, RT::Stone, RT::Glass, RT::Papyrus };
				std::sort(std::begin(allResources), std::end(allResources), [&](ResourceType _1, ResourceType _2) { return goldCostPerResource[u32(_1)] > goldCostPerResource[u32(_2)]; });

				u32 freeResources = 2;
				for (ResourceType r : allResources)
				{
					while (freeResources > 0 && cardResourceCost[u32(r)] > 0)
					{
						cardResourceCost[u32(r)]--;
						freeResources--;
					}
				}
			}

			// first spend normal resource
			for (u32 i = 0; i < m_weakProduction.first; ++i)
			{
				for (ResourceType r : normalResources)
				{
					if (cardResourceCost[u32(r)] > 0)
					{
						cardResourceCost[u32(r)]--;
						break;
					}
				}
			}

			// then same for rare resource
			for (u32 i = 0; i < m_weakProduction.second; ++i)
			{
				for (ResourceType r : rareResources)
				{
					if (cardResourceCost[u32(r)] > 0)
					{
						cardResourceCost[u32(r)]--;
						break;
					}
				}
			}

			u32 finalCost = 0;
			for (u32 i = 0; i < u32(RT::Count); ++i)
				finalCost += cardResourceCost[i] * goldCostPerResource[i];

			return finalCost + _card.m_goldCost;
		}
		return _card.m_goldCost;
	}

	SpecialAction PlayerCity::addCard(const Card& _card, const PlayerCity& _otherCity)
	{
		SpecialAction action = SpecialAction::Nothing;

		if (_card.m_chainIn != ChainingSymbol::None && m_chainingSymbols & (1u << u32(_card.m_chainIn)) && ownScienceToken(ScienceToken::TownPlanning))
			m_gold += 4;

		m_chainingSymbols |= (1u << u32(_card.m_chainOut));

		if (_card.m_goldPerNumberOfCardColorTypeCard)
			m_gold += m_numCardPerType[_card.m_secondaryType] * _card.m_goldReward;
		else if(_card.m_type == CardType::Guild && _card.m_secondaryType < u32(CardType::Count))
			m_gold += std::max(m_numCardPerType[_card.m_secondaryType], _otherCity.m_numCardPerType[_card.m_secondaryType]) * _card.m_goldReward;
		else
			m_gold += _card.m_goldReward;

		if (_card.m_type == CardType::Brown || _card.m_type == CardType::Grey)
		{
			for (u32 type = 0; type < u32(ResourceType::Count); ++type)
			{
				if (_card.m_production[type] > 0)
				{
					if (m_bestProductionCardId[type] == u8(-1) ||
						_card.m_production[type] > m_context->getCard(m_bestProductionCardId[type]).m_production[type])
					{
						m_bestProductionCardId[type] = _card.getId();
					}
				}
			}
		}

		m_numCardPerType[u32(_card.m_type)]++;
		m_victoryPoints += _card.m_victoryPoints;

		for (u32 i = 0; i < u32(ResourceType::Count); ++i)
		{
			if (_card.m_isResourceDiscount)
				m_resourceDiscount[i] |= _card.m_production[i] > 0;
			else if (_card.m_isWeakProduction); 
				// handled differently
			else
				m_production[i] += _card.m_production[i];
		}

		if (_card.m_isWeakProduction)
		{
			DEBUG_ASSERT(_card.m_production[u32(RT::Wood)] == _card.m_production[u32(RT::Clay)] && 
						 _card.m_production[u32(RT::Wood)] == _card.m_production[u32(RT::Stone)]);
			DEBUG_ASSERT(_card.m_production[u32(RT::Glass)] == _card.m_production[u32(RT::Papyrus)]);

			m_weakProduction.first += _card.m_production[u32(RT::Wood)];
			m_weakProduction.second += _card.m_production[u32(RT::Glass)];
		}

		switch (_card.m_type)
		{
		case CardType::Science:
			if (m_ownedScienceSymbols & (1u << u32(_card.m_science)))
				action = SpecialAction::TakeScienceToken;
			else
			{
				m_ownedScienceSymbols |= 1u << u32(_card.m_science);
				m_numScienceSymbols++;
			}
			break;

		case CardType::Guild:
			m_ownedGuildCards |= (1u << _card.m_secondaryType);
			break;

		case CardType::ScienceToken:
			if(ScienceToken(_card.m_secondaryType) == ScienceToken::Mathematics)
				m_victoryPoints += 3 * u8(core::countBits(m_ownedScienceTokens));

			if (ScienceToken(_card.m_secondaryType) == ScienceToken::Law)
			{
				m_ownedScienceSymbols |= 1u << u32(ScienceSymbol::Law);
				m_numScienceSymbols++;
			}

			m_ownedScienceTokens |= 1u << _card.m_secondaryType;
			if (ownScienceToken(ScienceToken::Mathematics))
				m_victoryPoints += 3;
			break;

		case CardType::Wonder:
			if (Helper::isReplayWonder(Wonders(_card.m_secondaryType)) || ownScienceToken(ScienceToken::Theology))
				action = SpecialAction::Replay;
			break;
		}
		
		if (m_numScienceSymbols == 6)
			return SpecialAction::ScienceWin;

		return action;
	}

	void PlayerCity::removeCard(const Card& _card)
	{
		DEBUG_ASSERT(_card.getType() == CardType::Brown || _card.getType() == CardType::Grey);
		DEBUG_ASSERT(_card.m_chainIn == ChainingSymbol::None && _card.m_chainOut == ChainingSymbol::None);

		for(u32 i=0 ; i < u32(ResourceType::Count) ; ++i)
			m_production[i] -= _card.m_production[i];
	}

	u32 PlayerCity::computeVictoryPoint(const PlayerCity& _otherCity) const
	{
		u32 goldVP = m_gold / 3;
		if (m_ownedGuildCards & (1 << u32(CardType::Count)))
			goldVP *= 2;

		u32 guildVP = 0;
		for (const Card& card : m_context->getAllGuildCards())
		{
			if (card.getSecondaryType() < u32(CardType::Count) && m_ownedGuildCards & (1 << card.getSecondaryType()))
			{
				u32 numCards = std::max(m_numCardPerType[card.getSecondaryType()], _otherCity.m_numCardPerType[card.getSecondaryType()]);
				guildVP += card.m_victoryPoints * numCards;
			}
		}

		return m_victoryPoints + goldVP + guildVP;
	}

	DiscardedCards::DiscardedCards()
	{
		for (u8& x : scienceCards)
			x = u8(-1);
		for (u8& x : guildCards)
			x = u8(-1);
	}

	void DiscardedCards::add(const GameContext& _context, const Card& _card)
	{
		if (_card.getMilitary() > 0)
		{
			if (militaryCard == u8(-1) || _card.getMilitary() > _context.getCard(militaryCard).getMilitary())
				militaryCard = _card.getId();
		}

		if (_card.m_victoryPoints > 0)
		{
			if (bestVictoryPoint == u8(-1) || _card.m_victoryPoints > _context.getCard(bestVictoryPoint).m_victoryPoints)
				bestVictoryPoint = _card.getId();
		}

		if (_card.getType() == CardType::Science)
		{
			u32 scienceIndex = u32(_card.m_science);
			if (scienceCards[scienceIndex] != u8(-1))
			{
				if(_card.m_victoryPoints > _context.getCard(bestVictoryPoint).m_victoryPoints)
					scienceCards[scienceIndex] = _card.getId();
			}
			else
				scienceCards[scienceIndex] = _card.getId();
		}

		if (_card.getType() == CardType::Guild)
			guildCards[numGuildCards++] = _card.getId();
	}

	template<typename T>
	u32 GameState::fillTensorData(T* _data, u32 _mainPlayer) const
	{
		u32 i = 0;
		_data[i++] = (T)m_numTurnPlayed;
		_data[i++] = (T)m_currentAge;
		_data[i++] = (T)(_mainPlayer == 0 ? m_military : -m_military);
		_data[i++] = (T)militaryToken2[_mainPlayer];
		_data[i++] = (T)militaryToken5[_mainPlayer];
		_data[i++] = (T)militaryToken2[(_mainPlayer + 1) % 2];
		_data[i++] = (T)militaryToken5[(_mainPlayer + 1) % 2];

		for (u32 j = 0; j < u32(ScienceToken::Count); ++j) {
			_data[i + j] = 0;
		}
		for (u32 j = 0; j < m_numScienceToken; ++j)
			_data[i + u32(m_scienceTokens[j])] = 1;

		i += u32(ScienceToken::Count);

		const PlayerCity& myCity = m_playerCity[_mainPlayer];
		const PlayerCity& opponentCity = m_playerCity[(_mainPlayer + 1) % 2];
		for (u8 j = 0; j < u8(Wonders::Count); ++j)
		{
			if (std::find(myCity.m_unbuildWonders.begin(), myCity.m_unbuildWonders.end(), (Wonders)j) != myCity.m_unbuildWonders.end())
				_data[i] = 1;
			else if (std::find(opponentCity.m_unbuildWonders.begin(), opponentCity.m_unbuildWonders.end(), (Wonders)j) != opponentCity.m_unbuildWonders.end())
				_data[i] = -1;
			else
				_data[i] = 0;
			
			i++;
		}
		

		auto fillCity = [&](const PlayerCity& _city)
		{
			for(u8 j=0 ; j<u8(ChainingSymbol::Count) ; ++j)
				_data[i++] = (T)((_city.m_chainingSymbols & (1 << j)) > 0 ? 1 : 0);

			for (size_t j=0 ; i<m_context->getAllGuildCards().size() ; ++j)
				_data[i++] = (T)((_city.m_ownedGuildCards & (1 << j)) > 0 ? 1 : 0);

			for (u8 j = 0; j < u8(ScienceToken::Count); ++j)
				_data[i++] = (T)((_city.m_ownedScienceTokens & (1 << j)) > 0 ? 1 : 0);

			_data[i++] = _city.m_numScienceSymbols;
			for (u8 j = 0; j < u8(ScienceSymbol::Count); ++j)
				_data[i++] = (T)((_city.m_ownedScienceSymbols & (1 << j)) > 0 ? 1 : 0);

			_data[i++] = (T)_city.m_gold;
			_data[i++] = (T)_city.m_victoryPoints;
			
			for (u8 j = 0; j < u8(CardType::Count); ++j)
			{
				_data[i++] = (T)_city.m_numCardPerType[j];
				_data[i++] = (T)(_city.m_resourceDiscount[j] ? 1 : 0);
			}

			for (u8 j = 0; j < u8(ResourceType::Count); ++j)
			{
				_data[i++] = (T)_city.m_production[j];
				_data[i++] = (T)_city.m_bestProductionCardId[j];
			}

			_data[i++] = (T)_city.m_weakProduction.first;
			_data[i++] = (T)_city.m_weakProduction.second;
		};

		fillCity(myCity);
		fillCity(opponentCity);

		return i;
	}

	template u32 GameState::fillTensorData<float>(float* _data, u32 _mainPlayer) const;
	template u32 GameState::fillTensorData<int16_t>(int16_t* _data, u32 _mainPlayer) const;
}
