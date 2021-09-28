#include "widgets/BlokusViewer.h"

#include "graphics/BoardGraphicsItem.h"

BlokusViewer::BlokusViewer(QWidget* _parent)
    : QGraphicsView(_parent)
    , m_boardViewer{ new BoardGraphicsItem }
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto scene = new QGraphicsScene(this);
    setScene(scene);
    scene->addItem(m_boardViewer);

    resize(900, 900);
}

void BlokusViewer::resizeEvent(QResizeEvent* _event)
{
    fitInView(m_boardViewer, Qt::KeepAspectRatio);

    QGraphicsView::resizeEvent(_event);
}

void BlokusViewer::setBoard(const blokusAI::Board& _board)
{ 
    m_board = _board;
    m_boardViewer->setBoard(&m_board);
}