#include "widgets/GameView.h"

#include "AI/BlokusGameHelpers.h"

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
        auto pieces = blokusAI::Helpers::getAllPieces();
        m_boardViewer->addPiece(pieces[5], blokusAI::Slot::P0, { 0, 0 });
        m_boardViewer->addPiece(pieces[7].rotate(blokusAI::Rotation::Rot_90), blokusAI::Slot::P1, { blokusAI::Board::BoardSize - 2, 0 });
        m_boardViewer->addPiece(pieces[9].rotate(blokusAI::Rotation::Rot_90), blokusAI::Slot::P2, { blokusAI::Board::BoardSize - 1, blokusAI::Board::BoardSize - 5 });
        m_boardViewer->addPiece(pieces[11].rotate(blokusAI::Rotation::Rot_270), blokusAI::Slot::P3, { 0, blokusAI::Board::BoardSize - 3 });
        m_boardViewer->addPiece(pieces[13], blokusAI::Slot::P0, { 2, 2 });
    }

    void GameView::resizeEvent(QResizeEvent* _event)
    {
        fitInView(m_boardViewer, Qt::KeepAspectRatio);

        BlokusGraphicsView::resizeEvent(_event);
    }

    void GameView::setBoard(const blokusAI::Board& _board)
    { 
        m_board = _board;
        m_boardViewer->setBoard(&m_board);
    }
}
