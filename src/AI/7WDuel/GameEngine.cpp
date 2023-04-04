#include "AI/7WDuel/GameEngine.h"
#include "AI/blockusAI/BlokusAI.h"

namespace sevenWD
{
	//----------------------------------------------------------------------------
	Card::Card(CardTag<CardType::Blue>, const char* _name, u8 _victoryPoints) : m_type{ CardType::Blue }, m_name { _name }, m_victoryPoints{ _victoryPoints }
	{
	}

	Card::Card(CardTag<CardType::Brown>, const char* _name, ResourceType _resource, u8 _num) : m_type{ CardType::Brown }, m_name{ _name }
	{
		m_production[u32(_resource)] = _num;
	}

	Card::Card(CardTag<CardType::Grey>, const char* _name, ResourceType _resource) : m_type{ CardType::Grey }, m_name{ _name }
	{
		m_production[u32(_resource)] = 1;
	}

	Card::Card(CardTag<CardType::Military>, const char* _name, u8 _numShields) : m_type{ CardType::Military }, m_name{ _name }, m_military{ _numShields }
	{
	}

	Card::Card(CardTag<CardType::Science>, const char* _name, ScienceSymbol _science, u8 _victoryPoints) : m_type{ CardType::Science }, m_name{ _name }, m_victoryPoints{ _victoryPoints }, m_science{ _science }
	{
	}

	Card::Card(CardTag<CardType::Guild>, const char* _name, CardType _cardColorForBonus, u8 _goldReward, u8 _victoryPointReward) : m_type{ CardType::Guild }, m_name{ _name }, m_victoryPoints{ _victoryPointReward }, m_goldReward{ _goldReward }
	{
		m_secondaryType = u8(_cardColorForBonus);
	}

	Card::Card(CardTag<CardType::Yellow>, const char* _name, u8 _victoryPoints) : m_type{ CardType::Yellow }, m_name{ _name }, m_victoryPoints{ _victoryPoints }
	{
	}

	Card::Card(Wonders _wonder, const char* _name, u8 _victoryPointReward, bool _extraTurn) : m_type{ CardType::Wonder }, m_name{ _name }, m_victoryPoints{ _victoryPointReward }, m_secondaryType{ u8(_wonder)}, m_extraTurn{_extraTurn}
	{
	}

	Card& Card::setResourceDiscount(ResourceSet _resources)
	{
		m_isResourceDiscount = true;
		m_isWeakProduction = false;

		for (u8& r : m_production)
			r = 0;

		for (ResourceType type : _resources)
			m_production[u32(type)]++;

		return *this;
	}

	Card& Card::setWeakResourceProduction(ResourceSet _resources)
	{
		m_isResourceDiscount = false;
		m_isWeakProduction = true;

		for (u8& r : m_production)
			r = 0;

		for (ResourceType type : _resources)
			m_production[u32(type)]++;

		return *this;
	}

	Card& Card::setGoldReward(u8 _reward)
	{
		m_isResourceDiscount = false;
		m_isWeakProduction = false;

		for (u8& r : m_production)
			r = 0;

		m_goldReward = _reward;
		return *this;
	}

	Card& Card::setGoldRewardForCardColorCount(u8 _gold, CardType _typeRewarded)
	{
		m_goldPerNumberOfCardColorTypeCard = true;
		m_goldReward = _gold;
		m_secondaryType = u8(_typeRewarded);
		return *this;
	}

	Card& Card::setChainIn(ChainingSymbol _symbol)
	{
		m_chainIn = _symbol;
		return *this;
	}

	Card& Card::setChainOut(ChainingSymbol _symbol)
	{
		m_chainOut = _symbol;
		return *this;
	}

	Card& Card::setResourceCost(ResourceSet _cost)
	{
		for (u8& cost : m_cost)
			cost = 0;

		for (ResourceType type : _cost)
			m_cost[u32(type)]++;

		return *this;
	}

	Card& Card::setGoldCost(u8 _num)
	{
		m_goldCost = _num;
		return *this;
	}

	Card& Card::setMilitary(u8 _shield)
	{
		m_military = _shield;
		return *this;
	}

	//----------------------------------------------------------------------------
	void Card::setId(u8 _id)
	{
		m_id = _id;
	}

	//----------------------------------------------------------------------------
	GameContext::GameContext(unsigned _seed) : m_rand(_seed)
	{
		m_allCards.clear();
		fillAge1();
		fillAge2();
		fillAge3();
		fillGuildCards();
		fillWonders();
	}

	//----------------------------------------------------------------------------
	void GameContext::initCityWithRandomWonders(PlayerCity& _player1, PlayerCity& _player2) const
	{
		std::vector<Wonders> wonders((u32)Wonders::Count);
		for (u32 i = 0; i < wonders.size(); ++i)
			wonders[i] = Wonders(i);

		std::shuffle(wonders.begin(), wonders.end(), m_rand);

		_player1.m_unbuildWonderCount = 4;
		for (u32 i = 0; i < 4; ++i)
			_player1.m_unbuildWonders[i] = wonders[i];

		_player2.m_unbuildWonderCount = 4;
		for (u32 i = 0; i < 4; ++i)
			_player2.m_unbuildWonders[i] = wonders[i];
	}

	void GameContext::initAge1Permutation(std::array<u8, 20>& _permut) const
	{
		std::array<u8, 23> age1Index;
		for (u32 i = 0; i < m_age1Cards.size(); ++i)
			age1Index[i] = m_age1Cards[i].getId();

		std::shuffle(age1Index.begin(), age1Index.end(), rand());
		std::copy(age1Index.begin(), age1Index.begin() + 20, _permut.begin());
	}

	void GameContext::fillAge1()
	{
		m_age1Cards.push_back(Card(CardTag<CardType::Blue>{}, "Autel", 3).setChainOut(ChainingSymbol::Moon));
		m_age1Cards.push_back(Card(CardTag<CardType::Blue>{}, "Bains", 3).setResourceCost({ RT::Stone }).setChainOut(ChainingSymbol::WaterDrop));
		m_age1Cards.push_back(Card(CardTag<CardType::Blue>{}, "Theater", 3).setChainOut(ChainingSymbol::Mask));

		m_age1Cards.push_back(Card(CardTag<CardType::Brown>{}, "Chantier",		 RT::Wood, 1));
		m_age1Cards.push_back(Card(CardTag<CardType::Brown>{}, "Exploitation",	 RT::Wood, 1).setGoldCost(1));
		m_age1Cards.push_back(Card(CardTag<CardType::Brown>{}, "BassinArgileux", RT::Clay, 1));
		m_age1Cards.push_back(Card(CardTag<CardType::Brown>{}, "Cavite",		 RT::Clay, 1).setGoldCost(1));
		m_age1Cards.push_back(Card(CardTag<CardType::Brown>{}, "Gisement",		 RT::Stone, 1));
		m_age1Cards.push_back(Card(CardTag<CardType::Brown>{}, "Mine",			 RT::Stone, 1).setGoldCost(1));

		m_age1Cards.push_back(Card(CardTag<CardType::Grey>{}, "Verrerie", RT::Glass).setGoldCost(1));
		m_age1Cards.push_back(Card(CardTag<CardType::Grey>{}, "Presse",	  RT::Papyrus).setGoldCost(1));
	     
		m_age1Cards.push_back(Card(CardTag<CardType::Yellow>{}, "Taverne",		0).setGoldReward(4).setChainOut(ChainingSymbol::Jar));
		m_age1Cards.push_back(Card(CardTag<CardType::Yellow>{}, "DepotBois",    0).setGoldCost(3).setResourceDiscount({ RT::Wood }));
		m_age1Cards.push_back(Card(CardTag<CardType::Yellow>{}, "DepotArgile",	0).setGoldCost(3).setResourceDiscount({ RT::Clay }));
		m_age1Cards.push_back(Card(CardTag<CardType::Yellow>{}, "DepotPierre",	0).setGoldCost(3).setResourceDiscount({ RT::Stone }));
					
		m_age1Cards.push_back(Card(CardTag<CardType::Military>{}, "TourDeGarde", 1));
		m_age1Cards.push_back(Card(CardTag<CardType::Military>{}, "Caserne",	 1).setResourceCost({ RT::Clay }).setChainOut(ChainingSymbol::Sword));
		m_age1Cards.push_back(Card(CardTag<CardType::Military>{}, "Ecurie",		 1).setResourceCost({ RT::Wood }).setChainOut(ChainingSymbol::Horseshoe));
		m_age1Cards.push_back(Card(CardTag<CardType::Military>{}, "Palissade",	 1).setGoldCost(2).setChainOut(ChainingSymbol::Tower));

		m_age1Cards.push_back(Card(CardTag<CardType::Science>{}, "Apothicaire",	ScienceSymbol::Wheel,	 1).setResourceCost({ RT::Glass }));
		m_age1Cards.push_back(Card(CardTag<CardType::Science>{}, "Atelier",		ScienceSymbol::Triangle, 1).setResourceCost({ RT::Papyrus }));
		m_age1Cards.push_back(Card(CardTag<CardType::Science>{}, "Scriptorium", ScienceSymbol::Script,	 0).setGoldCost(2).setChainOut(ChainingSymbol::Book));
		m_age1Cards.push_back(Card(CardTag<CardType::Science>{}, "Officine",	ScienceSymbol::Bowl,	 0).setGoldCost(2).setChainOut(ChainingSymbol::Gear));

		for (Card& card : m_age1Cards)
		{
			card.setId(u8(m_allCards.size()));
			m_allCards.push_back(&card);
		}
	}

	void GameContext::fillAge2()
	{
		m_age2Cards.push_back(Card(CardTag<CardType::Blue>{}, "Tribunal", 5).setResourceCost({ RT::Wood, RT::Wood, RT::Glass }));
		m_age2Cards.push_back(Card(CardTag<CardType::Blue>{}, "Statue",	  4).setResourceCost({ RT::Clay, RT::Clay }).setChainIn(ChainingSymbol::Mask).setChainOut(ChainingSymbol::GreekPillar));
		m_age2Cards.push_back(Card(CardTag<CardType::Blue>{}, "Temple",   4).setResourceCost({ RT::Wood, RT::Papyrus }).setChainIn(ChainingSymbol::Moon).setChainOut(ChainingSymbol::Sun));
		m_age2Cards.push_back(Card(CardTag<CardType::Blue>{}, "Aqueduc",  5).setResourceCost({ RT::Stone, RT::Stone, RT::Stone }).setChainIn(ChainingSymbol::WaterDrop));
		m_age2Cards.push_back(Card(CardTag<CardType::Blue>{}, "Rostres",  4).setResourceCost({ RT::Stone, RT::Wood }).setChainOut(ChainingSymbol::Bank));

		m_age2Cards.push_back(Card(CardTag<CardType::Brown>{}, "Scierie", RT::Wood, 2).setGoldCost(2));
		m_age2Cards.push_back(Card(CardTag<CardType::Brown>{}, "Briquerie", RT::Clay, 2).setGoldCost(2));
		m_age2Cards.push_back(Card(CardTag<CardType::Brown>{}, "Carriere", RT::Stone, 2).setGoldCost(2));

		m_age2Cards.push_back(Card(CardTag<CardType::Grey>{}, "Soufflerie", RT::Glass));
		m_age2Cards.push_back(Card(CardTag<CardType::Grey>{}, "Sechoire", RT::Papyrus));

		m_age2Cards.push_back(Card(CardTag<CardType::Yellow>{}, "Brasserie", 0).setGoldReward(6).setChainOut(ChainingSymbol::Barrel));
		m_age2Cards.push_back(Card(CardTag<CardType::Yellow>{}, "Caravanserail", 0).setGoldCost(2).setResourceCost({ RT::Glass, RT::Papyrus }).setWeakResourceProduction({ RT::Wood, RT::Clay, RT::Stone }));
		m_age2Cards.push_back(Card(CardTag<CardType::Yellow>{}, "Forum", 0).setGoldCost(3).setResourceCost({ RT::Clay }).setWeakResourceProduction({ RT::Glass, RT::Papyrus }));
		m_age2Cards.push_back(Card(CardTag<CardType::Yellow>{}, "Douane", 0).setGoldCost(4).setResourceDiscount({ RT::Papyrus, RT::Glass }));

		m_age2Cards.push_back(Card(CardTag<CardType::Military>{}, "Haras", 1).setResourceCost({ RT::Clay, RT::Wood }).setChainIn(ChainingSymbol::Horseshoe));
		m_age2Cards.push_back(Card(CardTag<CardType::Military>{}, "Baraquements", 1).setGoldCost(3).setChainIn(ChainingSymbol::Sword));
		m_age2Cards.push_back(Card(CardTag<CardType::Military>{}, "ChampsDeTir", 2).setResourceCost({ RT::Stone, RT::Wood, RT::Papyrus }).setChainOut(ChainingSymbol::Target));
		m_age2Cards.push_back(Card(CardTag<CardType::Military>{}, "PlaceArmes", 2).setResourceCost({ RT::Clay, RT::Clay, RT::Glass }).setChainOut(ChainingSymbol::Helmet));
		m_age2Cards.push_back(Card(CardTag<CardType::Military>{}, "Muraille", 2).setResourceCost({ RT::Stone, RT::Stone }));

		m_age2Cards.push_back(Card(CardTag<CardType::Science>{}, "Ecole", ScienceSymbol::Wheel, 1).setResourceCost({ RT::Wood, RT::Papyrus, RT::Papyrus }).setChainOut(ChainingSymbol::Harp));
		m_age2Cards.push_back(Card(CardTag<CardType::Science>{}, "Laboratoire", ScienceSymbol::Triangle, 1).setResourceCost({ RT::Wood, RT::Glass, RT::Glass }).setChainOut(ChainingSymbol::Lamp));
		m_age2Cards.push_back(Card(CardTag<CardType::Science>{}, "Bibliotheque", ScienceSymbol::Script, 2).setResourceCost({ RT::Stone, RT::Wood, RT::Glass }).setChainIn(ChainingSymbol::Book));
		m_age2Cards.push_back(Card(CardTag<CardType::Science>{}, "Dispensaire", ScienceSymbol::Bowl, 2).setResourceCost({ RT::Clay, RT::Clay, RT::Stone }).setChainIn(ChainingSymbol::Gear));

		for (Card& card : m_age2Cards)
		{
			card.setId(u8(m_allCards.size()));
			m_allCards.push_back(&card);
		}
	}

	void GameContext::fillAge3()
	{
		m_age3Cards.push_back(Card(CardTag<CardType::Blue>{}, "Senat", 5).setResourceCost({ RT::Clay, RT::Clay, RT::Stone, RT::Papyrus }).setChainIn(ChainingSymbol::Bank));
		m_age3Cards.push_back(Card(CardTag<CardType::Blue>{}, "Obelisque", 5).setResourceCost({ RT::Stone, RT::Stone, RT::Glass }));
		m_age3Cards.push_back(Card(CardTag<CardType::Blue>{}, "Jardins", 6).setResourceCost({ RT::Clay, RT::Clay, RT::Wood, RT::Wood }).setChainIn(ChainingSymbol::GreekPillar));
		m_age3Cards.push_back(Card(CardTag<CardType::Blue>{}, "Pantheon", 6).setResourceCost({ RT::Clay, RT::Wood, RT::Papyrus, RT::Papyrus }).setChainIn(ChainingSymbol::Sun));
		m_age3Cards.push_back(Card(CardTag<CardType::Blue>{}, "Palace", 7).setResourceCost({ RT::Clay, RT::Stone, RT::Wood, RT::Glass, RT::Glass }));
		m_age3Cards.push_back(Card(CardTag<CardType::Blue>{}, "HotelDeVille", 7).setResourceCost({ RT::Stone, RT::Stone, RT::Stone, RT::Wood, RT::Wood }));

		m_age3Cards.push_back(Card(CardTag<CardType::Military>{}, "Fortifications", 2).setResourceCost({ RT::Stone, RT::Stone, RT::Clay, RT::Papyrus }).setChainIn(ChainingSymbol::Tower));
		m_age3Cards.push_back(Card(CardTag<CardType::Military>{}, "Cirque", 2).setResourceCost({ RT::Clay, RT::Clay, RT::Stone, RT::Stone }).setChainIn(ChainingSymbol::Helmet));
		m_age3Cards.push_back(Card(CardTag<CardType::Military>{}, "AtelierDeSiege", 2).setResourceCost({ RT::Wood, RT::Wood, RT::Wood, RT::Glass }).setChainIn(ChainingSymbol::Target));
		m_age3Cards.push_back(Card(CardTag<CardType::Military>{}, "Arsenal", 3).setResourceCost({ RT::Clay, RT::Clay, RT::Clay, RT::Wood, RT::Wood }));
		m_age3Cards.push_back(Card(CardTag<CardType::Military>{}, "Pretoire", 3).setGoldCost(8));

		m_age3Cards.push_back(Card(CardTag<CardType::Yellow>{}, "Armurerie", 3).setResourceCost({ RT::Stone, RT::Stone, RT::Glass }).setGoldRewardForCardColorCount(1, CardType::Military));
		m_age3Cards.push_back(Card(CardTag<CardType::Yellow>{}, "Phare", 3).setResourceCost({ RT::Clay, RT::Clay, RT::Glass }).setGoldRewardForCardColorCount(1, CardType::Yellow).setChainIn(ChainingSymbol::Jar));
		m_age3Cards.push_back(Card(CardTag<CardType::Yellow>{}, "Port", 3).setResourceCost({ RT::Wood, RT::Glass, RT::Papyrus }).setGoldRewardForCardColorCount(2, CardType::Brown));
		m_age3Cards.push_back(Card(CardTag<CardType::Yellow>{}, "ChambreDeCommerce", 3).setResourceCost({ RT::Papyrus, RT::Papyrus }).setGoldRewardForCardColorCount(3, CardType::Grey));
		m_age3Cards.push_back(Card(CardTag<CardType::Yellow>{}, "Arene", 3).setResourceCost({ RT::Clay, RT::Stone, RT::Wood }).setGoldRewardForCardColorCount(2, CardType::Wonder).setChainIn(ChainingSymbol::Barrel));

		m_age3Cards.push_back(Card(CardTag<CardType::Science>{}, "Observatoire", ScienceSymbol::Globe, 2).setResourceCost({ RT::Stone, RT::Papyrus, RT::Papyrus }).setChainIn(ChainingSymbol::Lamp));
		m_age3Cards.push_back(Card(CardTag<CardType::Science>{}, "University", ScienceSymbol::Globe, 2).setResourceCost({ RT::Clay, RT::Glass, RT::Papyrus }).setChainIn(ChainingSymbol::Harp));
		m_age3Cards.push_back(Card(CardTag<CardType::Science>{}, "Etude", ScienceSymbol::SolarClock, 3).setResourceCost({ RT::Wood, RT::Wood, RT::Glass, RT::Papyrus }));
		m_age3Cards.push_back(Card(CardTag<CardType::Science>{}, "Academie", ScienceSymbol::SolarClock, 3).setResourceCost({ RT::Stone, RT::Wood, RT::Glass, RT::Glass }));

		for (Card& card : m_age3Cards)
		{
			card.setId(u8(m_allCards.size()));
			m_allCards.push_back(&card);
		}
	}

	void GameContext::fillGuildCards()
	{
		m_guildCards.push_back(Card(CardTag<CardType::Guild>{}, "GuildeDesArmateurs", CardType::Brown, 1, 1).setResourceCost({ RT::Clay, RT::Stone, RT::Glass, RT::Papyrus }));
		m_guildCards.push_back(Card(CardTag<CardType::Guild>{}, "GuildeDesCommercant", CardType::Yellow, 1, 1).setResourceCost({ RT::Clay, RT::Wood, RT::Glass, RT::Papyrus }));
		m_guildCards.push_back(Card(CardTag<CardType::Guild>{}, "GuildeDesTacticiens", CardType::Military, 1, 1).setResourceCost({ RT::Stone, RT::Stone, RT::Clay, RT::Papyrus }));
		m_guildCards.push_back(Card(CardTag<CardType::Guild>{}, "GuildeDesMagistrats", CardType::Blue, 1, 1).setResourceCost({ RT::Wood, RT::Wood, RT::Clay, RT::Papyrus }));
		m_guildCards.push_back(Card(CardTag<CardType::Guild>{}, "GuildeDesSciences", CardType::Science, 1, 1).setResourceCost({ RT::Clay, RT::Clay, RT::Wood, RT::Wood }));
		m_guildCards.push_back(Card(CardTag<CardType::Guild>{}, "GuildeDesBatisseurs", CardType::Wonder, 0, 2).setResourceCost({ RT::Stone, RT::Stone, RT::Clay, RT::Wood, RT::Glass }));
		m_guildCards.push_back(Card(CardTag<CardType::Guild>{}, "GuildeDesUsuriers", CardType::Count, 3, 1).setResourceCost({ RT::Stone, RT::Stone, RT::Wood, RT::Wood }));

		for (Card& card : m_guildCards)
		{
			card.setId(u8(m_allCards.size()));
			m_allCards.push_back(&card);
		}
	}

	void GameContext::fillWonders()
	{
		m_wonders.resize(u32(Wonders::Count));

		m_wonders[u32(Wonders::Coloss)] = Card(Wonders::Coloss, "LeColosse", 3).setMilitary(2).setResourceCost({RT::Clay, RT::Clay, RT::Clay, RT::Glass});
		m_wonders[u32(Wonders::Atremis)] = Card(Wonders::Atremis, "TempleArtemis", 0, true).setGoldReward(12).setResourceCost({ RT::Wood, RT::Stone, RT::Glass, RT::Papyrus });
		m_wonders[u32(Wonders::Pyramids)] = Card(Wonders::Pyramids, "LesPyramides", 9).setResourceCost({ RT::Papyrus, RT::Stone, RT::Stone, RT::Stone });
		m_wonders[u32(Wonders::Zeus)] = Card(Wonders::Zeus, "StatueDeZeus", 3).setMilitary(1).setResourceCost({ RT::Papyrus, RT::Papyrus, RT::Clay, RT::Wood, RT::Stone });
		m_wonders[u32(Wonders::GreatLighthouse)] = Card(Wonders::GreatLighthouse, "LeGrandPhare", 4).setWeakResourceProduction({ RT::Clay, RT::Stone, RT::Wood }).setResourceCost({ RT::Papyrus, RT::Papyrus, RT::Stone, RT::Wood });
		m_wonders[u32(Wonders::CircusMaximus)] = Card(Wonders::CircusMaximus, "CircusMaximus", 3).setMilitary(1).setResourceCost({ RT::Stone, RT::Stone, RT::Wood, RT::Glass });

		m_wonders[u32(Wonders::GreatLibrary)] = Card(Wonders::GreatLibrary, "GreatLibrary", 4).setResourceCost({ RT::Wood, RT::Wood, RT::Wood, RT::Glass, RT::Papyrus });
		m_wonders[u32(Wonders::Sphinx)] = Card(Wonders::Sphinx, "Sphinx", 6, true).setResourceCost({ RT::Stone, RT::Clay, RT::Glass, RT::Glass });
		m_wonders[u32(Wonders::ViaAppia)] = Card(Wonders::ViaAppia, "LaViaAppia", 3, true).setGoldReward(3).setResourceCost({ RT::Clay, RT::Clay, RT::Stone, RT::Stone, RT::Papyrus });
		m_wonders[u32(Wonders::Piraeus)] = Card(Wonders::Piraeus, "LaPiree", 2, true).setWeakResourceProduction({ RT::Papyrus, RT::Glass }).setResourceCost({ RT::Clay, RT::Stone, RT::Wood, RT::Wood });
		m_wonders[u32(Wonders::HangingGarden)] = Card(Wonders::HangingGarden, "JardinSuspendus", 3, true).setGoldCost(6).setResourceCost({ RT::Papyrus, RT::Glass, RT::Wood, RT::Wood });
		m_wonders[u32(Wonders::Mausoleum)] = Card(Wonders::Mausoleum, "Mausoleum", 2).setResourceCost({ RT::Papyrus, RT::Glass, RT::Glass, RT::Clay, RT::Clay });

		for (Card& card : m_wonders)
		{
			card.setId(u8(m_allCards.size()));
			m_allCards.push_back(&card);
		}
	}

	//----------------------------------------------------------------------------
	GameState::GameState(const GameContext& _context) : m_context{ _context }
	{
		initScienceTokens();
		initAge1Graph();
		
		m_context.initCityWithRandomWonders(m_playerCity[0], m_playerCity[1]);
		m_context.initAge1Permutation(m_cardsPermutation);
	}

	SpecialAction GameState::pick(u32 _playableCardIndex)
	{
		u8 pickedCard = m_playableCards[_playableCardIndex];
		std::swap(m_playableCards[_playableCardIndex], m_playableCards[m_numPlayableCards - 1]);
		m_numPlayableCards--;

		unlinkNodeFromGraph(pickedCard);

		return m_playerCity[m_playerTurn].addCard(m_context.getCard(m_cardsPermutation[pickedCard]), m_playerCity[(m_playerTurn + 1) % 2]);
	}

	u32 GameState::genPyramidGraph(u32 _numRow, u32 _startNodeIndex)
	{
		u32 prevRowStartIndex = u32(-1);
		u32 curNodeIndex = _startNodeIndex;

		for (u32 row = 0; row < _numRow; ++row)
		{
			for (u32 i = 0; i < 2 + row; ++i)
			{
				m_graph[curNodeIndex + i].m_visible = row % 2 == 0;
				m_graph[curNodeIndex + i].m_child0 = CardNode::InvalidNode;
				m_graph[curNodeIndex + i].m_child1 = CardNode::InvalidNode;
				m_graph[curNodeIndex + i].m_unused = 0;

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
				m_graph[curNodeIndex + i].m_visible = row % 2 == 0;
				m_graph[curNodeIndex + i].m_child0 = CardNode::InvalidNode;
				m_graph[curNodeIndex + i].m_child1 = CardNode::InvalidNode;
				m_graph[curNodeIndex + i].m_unused = 0;

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
					m_graph[parent].m_visible = true;
					m_playableCards[m_numPlayableCards++] = parent;
				}
			}
		};
		removeFromParent(m_graph[_nodeIndex].m_parent0);
		removeFromParent(m_graph[_nodeIndex].m_parent1);
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

		std::shuffle(std::begin(m_scienceTokens), std::end(m_scienceTokens), m_context.rand());
		m_numScienceToken = 5; // 5 first tokens are used on the board
	}

	void GameState::initAge1Graph()
	{
		u32 end = genPyramidGraph(5, 0);

		m_numPlayableCards = 0;
		for (u32 i = 0; i < 6; ++i)
			m_playableCards[m_numPlayableCards++] = u8(end - 6 + i);
	}

	void GameState::initAge2Graph()
	{
		u32 end = genInversePyramidGraph(6, 5, 0);

		m_numPlayableCards = 0;
		for (u32 i = 0; i < 2; ++i)
			m_playableCards[m_numPlayableCards++] = u8(end - 2 + i);
	}

	void GameState::initAge3Graph()
	{
		u32 end = genPyramidGraph(3, 0);

		const u32 connectNode0 = end;
		const u32 connectNode1 = end + 1;

		m_graph[connectNode0].m_visible = 0;
		m_graph[connectNode1].m_visible = 0;

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

		m_numPlayableCards = 0;
		for (u32 i = 0; i < 2; ++i)
			m_playableCards[m_numPlayableCards++] = u8(end - 2 + i);
	}

	u32 PlayerCity::computeCost(const Card& _card, const PlayerCity& _otherPlayer)
	{
		if (m_chainingSymbols & (1u << u32(_card.m_chainIn)))
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
			cardResourceCost[i] = m_production[i] > cardResourceCost[i] ? 0 : cardResourceCost[i] - m_production[i];
			empty &= cardResourceCost[i] == 0;
		}
		
		if (!empty)
		{
			ResourceType normalResources[] = { RT::Wood, RT::Clay, RT::Stone };
			ResourceType rareResources[] = { RT::Glass, RT::Papyrus };
			std::sort(std::begin(normalResources), std::end(normalResources), [&](ResourceType _1, ResourceType _2) { return goldCostPerResource[u32(_1)] > goldCostPerResource[u32(_2)]; });

			if (goldCostPerResource[u32(rareResources[1])] > goldCostPerResource[u32(rareResources[0])])
				std::swap(rareResources[0], rareResources[1]);

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

		m_chainingSymbols |= (1u << u32(_card.m_chainOut));

		if (_card.m_goldPerNumberOfCardColorTypeCard)
			m_gold += m_numCardPerType[_card.m_secondaryType] * _card.m_goldReward;
		else if(_card.m_type == CardType::Guild)
			m_gold += std::max(m_numCardPerType[_card.m_secondaryType], _otherCity.m_numCardPerType[_card.m_secondaryType]) * _card.m_goldReward;
		else
			m_gold += _card.m_goldReward;

		if (_card.m_type == CardType::Brown)
			m_brownCardIndexes[m_numCardPerType[u32(CardType::Brown)]] = u8(_card.m_id);
		if (_card.m_type == CardType::Grey)
			m_greyCardIndexes[m_numCardPerType[u32(CardType::Grey)]] = u8(_card.m_id);

		m_numCardPerType[u32(_card.m_type)]++;
		m_victoryPoints += m_victoryPoints;

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
			m_ownedScienceSymbols |=  1u << u32(_card.m_science);
			break;

		case CardType::Guild:
			m_ownedGuildCards |= 1u << _card.m_secondaryType;
			break;

		case CardType::Wonder:
		{
			auto it = std::remove(std::begin(m_unbuildWonders), std::begin(m_unbuildWonders) + m_unbuildWonderCount, Wonders(_card.m_secondaryType));
			DEBUG_ASSERT(it != std::end(m_unbuildWonders));
			size_t index = std::distance(std::begin(m_unbuildWonders), it);
			std::swap(m_unbuildWonders[m_unbuildWonderCount-1], m_unbuildWonders[index]);
			m_unbuildWonderCount--;
			
			// Apply Wonder speical effect
			//switch (Wonders(_card.m_secondaryType))
			//{
			//	case 
			//}
			break;
		}
		}
		
		return action;
	}
}
