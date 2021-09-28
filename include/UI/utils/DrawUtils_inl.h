#pragma once

#include "AI/BlokusGame.h"
#include "UI/game/GameConstants.h"

namespace blokusUI
{
    constexpr QRectF DrawUtils::getBoardBoundingRect()
    {
        constexpr qreal size = (blokusAI::Board::BoardSize + BoardBorderWidthRatio * 2) * GameConstants::TileSizeScale;
        return QRectF{ 0, 0, size, size };
    }

    constexpr QPointF DrawUtils::getBoardOffset()
    {
        constexpr qreal cornerThickness = BoardBorderWidthRatio * GameConstants::TileSizeScale;
        return { cornerThickness, cornerThickness };
    }

    constexpr QPointF DrawUtils::getTileOffset(u32 _x, u32 _y)
    {
        constexpr QPointF baseOffset{ getBoardOffset() };
        return QPointF{
            baseOffset.x() + _x * GameConstants::TileSizeScale,
            baseOffset.y() + _y * GameConstants::TileSizeScale };
    }
}
