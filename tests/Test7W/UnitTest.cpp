#include "AI/7WDuel/GameEngine.h"
#include "Core/Common.h"

namespace sevenWD
{
	void costTest()
	{
		GameContext context;

		{
			Card card(CardTag<CardType::Blue>{}, "Test", 3);
			card.setResourceCost({ RT::Wood, RT::Clay, RT::Glass, RT::Papyrus });
			card.setGoldCost(10);

			PlayerCity opoCity(context);
			opoCity.m_production[u32(RT::Glass)] = 1;

			PlayerCity myCity(context);
			myCity.m_weakProduction.second = 1;
			myCity.m_production[u32(RT::Wood)] = 1;
			myCity.m_resourceDiscount[u32(RT::Clay)] = true;

			u32 cost = myCity.computeCost(card, opoCity);
			DEBUG_ASSERT(cost == 13);
		}

		{
			Card card(CardTag<CardType::Blue>{}, "Test", 3);
			card.setResourceCost({ RT::Wood, RT::Clay, RT::Stone });

			PlayerCity opoCity(context);
			opoCity.m_production[u32(RT::Stone)] = 3;
			opoCity.m_production[u32(RT::Clay)] = 2;
			opoCity.m_production[u32(RT::Wood)] = 1;

			PlayerCity myCity(context);
			myCity.m_weakProduction.first = 1;
			myCity.m_resourceDiscount[u32(RT::Stone)] = true;

			u32 cost = myCity.computeCost(card, opoCity);
			DEBUG_ASSERT(cost == 4);
		}
	}
}
