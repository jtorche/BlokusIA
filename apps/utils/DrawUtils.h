#pragma once

class QBrush;
class QColor;
class QPainter;

namespace blokusUi
{
    struct DrawUtils
	{
		static void drawTile(QPainter& _painter, const QColor& _color);
		static void drawTile(QPainter& _painter, const QBrush& _brush);
	};
}
