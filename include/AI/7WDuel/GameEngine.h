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
		Count
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
		Balance,
		Globe,
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
		Card(Wonders _wonders, const char* _name, u8 _victoryPointReward, bool _extraTurn = false);

		
		Card& setResourceDiscount(ResourceSet _resources);
		Card& setWeakResourceProduction(ResourceSet _resources);
		Card& setMilitary(u8 _shield);
		Card& setGoldReward(u8 _reward);
		Card& setGoldRewardForCardColorCount(u8 _gold, CardType _typeRewarded);
		
		Card& setChainIn(ChainingSymbol _symbol);
		Card& setChainOut(ChainingSymbol _symbol);
		Card& setResourceCost(ResourceSet _cost);
		Card& setGoldCost(u8 _num);

	private:
		const char* m_name = nullptr;
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
	};

	//----------------------------------------------------------------------------
	class GameContext
	{
	public:
		GameContext(unsigned _seed = 42);

		std::default_random_engine& rand() const { return m_rand; }

	private:
		mutable std::default_random_engine m_rand;
		std::vector<Card> m_age1Cards;
		std::vector<Card> m_age2Cards;
		std::vector<Card> m_age3Cards;
		std::vector<Card> m_guildCards;
		std::vector<Card> m_wonders;

	private:
		void fillAge1();
		void fillAge2();
		void fillAge3();
		void fillGuildCards();
		void fillWonders();
	};

	//----------------------------------------------------------------------------
	struct PlayerCity
	{
		u32 m_chainingSymbols = 0; // bitfield
		u32 m_ownedScienceTokens = 0; // bitfield
		u8 m_ownedScienceSymbols = 0; // bitfield
		u8 m_ownedGuildCards = 0; // bitfield
		u8 m_gold = 0;
		u8 m_victoryPoints = 0;
		std::array<u8, u32(CardType::Count)> m_numCardPerType = {};
		std::array<u8, u32(CardType::Count)> m_production = {};
		std::pair<u8, u8> m_weakProduction = {};
		std::array<bool, u32(CardType::Count)> m_resourceDiscount = {};
		std::array<Wonders, 4> m_unbuildWonders = {};
		u8 m_unbuildWonderCount = 4;

		u32 computeCost(const Card& _card, const PlayerCity& _otherPlayer);
	};

	//----------------------------------------------------------------------------
	class GameState
	{
	public:
		GameState(const GameContext& _context);

	private:
		
		struct CardNode
		{
			static constexpr u32 InvalidNode = 0x3F;

			u32 m_parent0 : 6;
			u32 m_parent1 : 6;
			u32 m_child0 : 6;
			u32 m_child1 : 6;
			u32 m_visible : 1;
			u32 m_unused : 7;
		};

		const GameContext& m_context;
		PlayerCity m_playerCity[2];
		std::array<ScienceToken, u8(ScienceToken::Count)> m_scienceTokens;
		u8 m_numScienceToken = 0;

		std::array<CardNode, 20> m_graph;
		std::array<u8, 23> m_cardsPermutation; // index in m_context
		std::array<u8, 6> m_playableCards; // index in m_graph
		u8 m_numPlayableCards;

		u8 m_currentAge = u8(-1);
		int8_t m_military = 0;

	private:
		u32 genPyramidGraph(u32 _numRow, u32 _startNodeIndex);
		u32 genInversePyramidGraph(u32 _baseSize, u32 _numRow, u32 _startNodeIndex);

		void initScienceTokens();
		void initAge1Graph();
		void initAge2Graph();
		void initAge3Graph();
	};
}