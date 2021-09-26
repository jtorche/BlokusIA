#include "widgets/GameView.h"

#include "IA/BlokusGameHelpers.h"

#include "game/BoardGraphicsItem.h"

namespace blokusUi
{
    GameView::GameView(QWidget* _parent)
        : BlokusGraphicsView(_parent)
        , m_boardViewer{ new BoardGraphicsItem(&m_board) }
    {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        auto scene = new QGraphicsScene(this);
        setScene(scene);
        scene->addItem(m_boardViewer);

        // Test pieces
        auto pieces = BlokusIA::Helpers::getAllPieces();
        m_boardViewer->addPiece(pieces[5], BlokusIA::Slot::P0, { 0, 0 });
        m_boardViewer->addPiece(pieces[7].rotate(BlokusIA::Rotation::Rot_90), BlokusIA::Slot::P1, { BlokusIA::Board::BoardSize - 2, 0 });
        m_boardViewer->addPiece(pieces[9].rotate(BlokusIA::Rotation::Rot_90), BlokusIA::Slot::P2, { BlokusIA::Board::BoardSize - 1, BlokusIA::Board::BoardSize - 5 });
        m_boardViewer->addPiece(pieces[11].rotate(BlokusIA::Rotation::Rot_270), BlokusIA::Slot::P3, { 0, BlokusIA::Board::BoardSize - 3 });
        m_boardViewer->addPiece(pieces[13], BlokusIA::Slot::P0, { 2, 2 });
    }

    void GameView::resizeEvent(QResizeEvent* _event)
    {
        fitInView(m_boardViewer, Qt::KeepAspectRatio);

        BlokusGraphicsView::resizeEvent(_event);
    }

    void GameView::setBoard(const BlokusIA::Board& _board)
    { 
        m_board = _board;
        m_boardViewer->setBoard(&m_board);
    }
}
