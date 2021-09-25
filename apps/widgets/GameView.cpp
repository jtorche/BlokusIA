#include "widgets/GameView.h"

#include "IA/BlokusGameHelpers.h"

#include "game/BoardGraphicsItem.h"

namespace blokusUi
{
    GameView::GameView(QWidget* _parent)
        : BlokusGraphicsView(_parent)
    {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        auto scene = new QGraphicsScene(this);
        setScene(scene);

        BlokusIA::Board boardModel;
        m_board = new BoardGraphicsItem(boardModel);
        scene->addItem(m_board);

        // Test pieces
        auto pieces = BlokusIA::Helpers::getAllPieces();
        m_board->addPiece(pieces[5], BlokusIA::Slot::P0, { 0, 0 });
        m_board->addPiece(pieces[7].rotate(BlokusIA::Rotation::Rot_90), BlokusIA::Slot::P1, { BlokusIA::Board::BoardSize - 2, 0 });
        m_board->addPiece(pieces[9].rotate(BlokusIA::Rotation::Rot_90), BlokusIA::Slot::P2, { BlokusIA::Board::BoardSize - 1, BlokusIA::Board::BoardSize - 5 });
        m_board->addPiece(pieces[11].rotate(BlokusIA::Rotation::Rot_270), BlokusIA::Slot::P3, { 0, BlokusIA::Board::BoardSize - 3 });
        m_board->addPiece(pieces[13], BlokusIA::Slot::P0, { 2, 2 });
    }

    void GameView::resizeEvent(QResizeEvent* _event)
    {
        fitInView(m_board, Qt::KeepAspectRatio);

        BlokusGraphicsView::resizeEvent(_event);
    }
}
