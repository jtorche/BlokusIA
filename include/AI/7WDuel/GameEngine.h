#pragma once

#include <array>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

#include "Core/Common.h"
#include "Core/hash.h"
#include "Core/type.h"

namespace sevenWD
{
	enum class ResourceType : u32
	{
		Wood = 0,
		Clay,
		Stone,
		Glass,
		Papyrus,
		Count,
		FirstBrown = Wood,
		LastBrown = Stone,
		FirstGrey = Glass,
		LastGrey = Papyrus
	};

	using RT = ResourceType;

	using ResourceSet = std::initializer_list<ResourceType>;

	enum class CardType : u8
	{
		Blue = 0,
		Brown,
		Grey,
		Yellow,
		Science,
		Military,
		Guild,
		Wonder,
		ScienceToken,
		Count
	};

	enum class Wonders : u8
	{
		CircusMaximus,
		Coloss,
		GreatLighthouse,
		HangingGarden,
		GreatLibrary,
		Mausoleum,
		Piraeus,
		Pyramids,
		Sphinx,
		Zeus,
		Atremis,
		ViaAppia,
		Count
	};

	enum class ScienceSymbol : u8
	{
		Wheel,
		Script,
		Triangle,
		Bowl,
		SolarClock,
		Globe,
		Law,
		Count
	};

	enum class ChainingSymbol : u8
	{
		None,
		Jar,
		Barrel,
		Mask,
		Bank,
		Sun,
		WaterDrop,
		GreekPillar,
		Moon,
		Target,
		Helmet,
		Horseshoe,
		Sword,
		Tower,
		Harp,
		Gear,
		Book,
		Lamp
	};

	template<CardType T>
	struct CardTag {};

	enum class ScienceToken : u8
	{
		Agriculture = 0,
		TownPlanning,
		Architecture,
		Theology,
		Strategy,
		Law,
		Mathematics,
		Masonry,    
		Philosophy,
		Economy,
		Count
	};

	enum class SpecialAction
	{
		Nothing,
		Replay,
		TakeScienceToken,
		MilitaryWin,
		ScienceWin
	};

	//----------------------------------------------------------------------------
	class Card
	{
	public:
		Card() = default;
		Card(CardTag<CardType::Blue>, const char * _name, u8 _victoryPoints);
		Card(CardTag<CardType::Brown>, const char* _name, ResourceType _resource, u8 _num);
		Card(CardTag<CardType::Grey>, const char* _name, ResourceType _resource);
		Card(CardTag<CardType::Military>, const char* _name, u8 _numShields);
		Card(CardTag<CardType::Yellow>, const char* _name, u8 _victoryPoints);
		Card(CardTag<CardType::Science>, const char* _name, ScienceSymbol _science, u8 _victoryPoints);
		Card(CardTag<CardType::Guild>, const char* _name, CardType _cardColorForBonus, u8 _goldReward, u8 _victoryPointReward);
		Card(ScienceToken _scienceToken, const char* _name, u8 _goldReward = 0, u8 _victoryPointReward = 0);
		Card(Wonders _wonders, const char* _name, u8 _victoryPointReward, bool _extraTurn = false);

		u8 getId() const { return m_id; }
		u8 getMilitary() const { return m_military; }

		CardType getType() const { return m_type; }
		u8 getSecondaryType() const { return m_secondaryType; }

		void setId(u8 _id);
		Card& setResourceDiscount(ResourceSet _resources);
		Card& setWeakResourceProduction(ResourceSet _resources);
		Card& setMilitary(u8 _shield);
		Card& setGoldReward(u8 _reward);
		Card& setGoldRewardForCardColorCount(u8 _gold, CardType _typeRewarded);
		
		Card& setChainIn(ChainingSymbol _symbol);
		Card& setChainOut(ChainingSymbol _symbol);
		Card& setResourceCost(ResourceSet _cost);
		Card& setGoldCost(u8 _num);

		const char* getName() const { return m_name; }
		void print() const;

	private:
		const char* m_name = nullptr;
		u8 m_id = u8(-1);
		CardType m_type = CardType::Count;
		
		ChainingSymbol m_chainIn = ChainingSymbol::None; 
		ChainingSymbol m_chainOut = ChainingSymbol::None;
		std::array<u8, u32(RT::Count)> m_production = {};
		u8 m_goldReward = 0;
		bool m_isWeakProduction = false;
		bool m_isResourceDiscount = false;
		
		std::array<u8, u32(RT::Count)> m_cost = {};
		u8 m_goldCost = 0;
		u8 m_victoryPoints = 0;
		u8 m_military = 0;
		ScienceSymbol m_science = ScienceSymbol::Count;
		bool m_goldPerNumberOfCardColorTypeCard = false; // special bit indicating that the card reward gold m_goldReward per number of card of type m_secondaryType
		bool m_extraTurn = false;
		u8 m_secondaryType = 0; // for additional effects (like guild cards or wonders)

		friend struct PlayerCity;
		friend struct DiscardedCards;
	};

	//----------------------------------------------------------------------------
	class GameContext
	{
	public:
		GameContext(unsigned _seed = 42);

		void initCityWithRandomWonders(PlayerCity& _player1, PlayerCity& _player2) const;

		std::default_random_engine& rand() const { return m_rand; }
		const Card& getCard(u8 _cardId) const { return *m_allCards[_cardId]; }
		const Card& getWonder(Wonders _wonder) const { return m_wonders[u32(_wonder)]; }
		const Card& getScienceToken(ScienceToken _token) const { return m_scienceTokens[u32(_token)]; }

		u8 getAge1CardCount() const { return u8(m_age1Cards.size()); }
		u8 getAge2CardCount() const { return u8(m_age2Cards.size()); }
		u8 getAge3CardCount() const { return u8(m_age3Cards.size()); }
		u8 getGuildCardCount() const { return u8(m_guildCards.size()); }

		const Card& getAge1Card(u32 _index) const { return m_age1Cards[_index]; }
		const Card& getAge2Card(u32 _index) const { return m_age2Cards[_index]; }
		const Card& getAge3Card(u32 _index) const { return m_age3Cards[_index]; }
		const Card& getGuildCard(u32 _index) const { return m_guildCards[_index]; }

		const std::vector<Card>& getAllGuildCards() const { return m_guildCards; }

	private:
		mutable std::default_random_engine m_rand;
		std::vector<Card> m_age1Cards;
		std::vector<Card> m_age2Cards;
		std::vector<Card> m_age3Cards;
		std::vector<Card> m_guildCards;
		std::vector<Card> m_wonders;
		std::vector<Card> m_scienceTokens;
		std::vector<const Card*> m_allCards;

	private:
		void fillAge1();
		void fillAge2();
		void fillAge3();
		void fillGuildCards();
		void fillWonders();
		void fillScienceTokens();
	};

	//----------------------------------------------------------------------------
	struct PlayerCity
	{
		const GameContext& m_context;
		u32 m_chainingSymbols = 0; // bitfield
		u16 m_ownedGuildCards = 0; // bitfield
		u16 m_ownedScienceTokens = 0; // bitfield
		u8 m_ownedScienceSymbols = 0; // bitfield
		u8 m_numScienceSymbols = 0;
		u8 m_gold = 0;
		u8 m_victoryPoints = 0;
		std::array<u8, u32(CardType::Count)> m_numCardPerType = {};
		std::array<u8, u32(ResourceType::Count)> m_production = {};
		std::pair<u8, u8> m_weakProduction = {};
		std::array<bool, u32(CardType::Count)> m_resourceDiscount = {};
		std::array<u8, u32(ResourceType::Count)> m_bestProductionCardId;
		std::array<Wonders, 4> m_unbuildWonders = {};
		u8 m_unbuildWonderCount = 4;

		PlayerCity(const GameContext& _context) : m_context{ _context } 
		{
			for (u8& index : m_bestProductionCardId)
				index = u8(-1);
		}

		u32 computeCost(const Card& _card, const PlayerCity& _otherPlayer) const;
		SpecialAction addCard(const Card& _card, const PlayerCity& _otherCity);
		void removeCard(const Card& _card);

		bool ownScienceToken(ScienceToken _token) const { return ((1u << u32(_token)) & m_ownedScienceTokens) > 0; }
		u32 computeVictoryPoint(const PlayerCity& _otherCity) const;

		void print();
	};

	struct DiscardedCards
	{
		u8 militaryCard = u8(-1);
		u8 bestVictoryPoint = u8(-1);
		u8 scienceCards[u32(ScienceSymbol::Count)];
		u8 guildCards[3];
		u8 numGuildCards = 0;

		DiscardedCards();
		void add(const GameContext&, const Card&);
	};

	//----------------------------------------------------------------------------
	class GameState
	{
		friend struct GameController;

	public:
		GameState(const GameContext& _context);

		enum class NextAge
		{
			None,
			Next,
			EndGame
		};
		u32 getCurrentAge() const { return m_currentAge; }
		NextAge nextAge();
		u32 getCurrentPlayerTurn() const { return m_playerTurn; }
		void nextPlayer() { m_playerTurn = (m_playerTurn + 1) % 2; }

		const Card& getPlayableCard(u32 _index) const;
		const Card& getPlayableScienceToken(u32 _index) const;
		const Card& getCurrentPlayerWonder(u32 _index) const;

		const PlayerCity& getPlayerCity(u32 _player) const { return m_playerCity[_player]; };

		std::array<ScienceToken, 5> getUnusedScienceToken() const;

		SpecialAction pick(u32 _playableCardIndex);
		void burn(u32 _playableCardIndex);
		SpecialAction buildWonder(u32 _withPlayableCardIndex, u32 _wondersIndex, u8 _additionalEffect = u8(-1));
		SpecialAction pickScienceToken(u32 _tokenIndex);
		u32 findWinner();

		void printGameState() const;
		void printPlayablCards() const;
		void printAvailableTokens() const;

	private:
		
		struct CardNode
		{
			static constexpr u32 InvalidNode = 0x1F;

			u32 m_parent0 : 5;
			u32 m_parent1 : 5;
			u32 m_child0 : 5;
			u32 m_child1 : 5;
			u32 m_cardId : 10;
			u32 m_visible : 1;
			u32 m_isGuildCard : 1;
		};

		const GameContext& m_context;
		std::array<PlayerCity, 2> m_playerCity;
		std::array<ScienceToken, u8(ScienceToken::Count)> m_scienceTokens;
		u8 m_numScienceToken = 0;

		std::array<CardNode, 20> m_graph;
		std::array<u8, 6> m_playableCards; // index in m_graph
		u8 m_numPlayableCards;

		std::array<u8, 23> m_availableAgeCards;
		std::array<u8, 7> m_availableGuildCards;
		u8 m_numAvailableAgeCards = 0;
		u8 m_numAvailableGuildCards = 0;

		u8 m_playerTurn = 0;
		u8 m_currentAge = u8(-1);
		int8_t m_military = 0;
		bool militaryToken2[2] = { false,false };
		bool militaryToken5[2] = { false,false };

	private:
		u32 genPyramidGraph(u32 _numRow, u32 _startNodeIndex);
		u32 genInversePyramidGraph(u32 _baseSize, u32 _numRow, u32 _startNodeIndex);

		PlayerCity& getCurrentPlayerCity() { return m_playerCity[m_playerTurn]; }
		const PlayerCity& getCurrentPlayerCity() const { return m_playerCity[m_playerTurn]; }
		const PlayerCity& getOtherPlayerCity() const { return m_playerCity[(m_playerTurn + 1) % 2]; }
		void unlinkNodeFromGraph(u32 _nodeIndex);

		void initScienceTokens();
		void initAge1Graph();
		void initAge2Graph();
		void initAge3Graph();

		template<typename T>
		u8 pickCardIndex(T& _availableCards, u8& _numAvailableCard);

		void pickCardAdnInitNode(CardNode& _node);
	};

	template<typename T>
	u8 GameState::pickCardIndex(T& _availableCards, u8& _numAvailableCard)
	{
		u32 index = m_context.rand()() % _numAvailableCard;

		u8 cardIndex = _availableCards[index];
		std::swap(_availableCards[index], _availableCards[_numAvailableCard - 1]);
		_numAvailableCard--;
		return cardIndex;
	}
}