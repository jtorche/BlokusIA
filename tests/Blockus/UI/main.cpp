#include <QApplication>

#include "AI/BlokusGameHelpers.h"

#include "widgets/BlokusViewer.h"

void compute(BlokusViewer& _viewer);

int main(int _argc, char* _argv[])
{
    QApplication app{ _argc, _argv };
    app.setApplicationName("Blokus AI Tests");
    app.setApplicationVersion("1.0.0");

    BlokusViewer viewer;
    viewer.show();

    compute(viewer);

    return app.exec();
}

void compute(BlokusViewer& _viewer)
{
    // Do some computation and ask for board display update
    auto pieces = blokusAI::Helpers::getAllPieces();
    blokusAI::Board board;
    board.addPiece(blokusAI::Slot::P0, pieces[5], { 0, 0 });
    board.addPiece(blokusAI::Slot::P1, pieces[7].rotate(blokusAI::Rotation::Rot_90), { blokusAI::Board::BoardSize - 2, 0 });
    board.addPiece(blokusAI::Slot::P2, pieces[9].rotate(blokusAI::Rotation::Rot_90), { blokusAI::Board::BoardSize - 1, blokusAI::Board::BoardSize - 5 });
    board.addPiece(blokusAI::Slot::P3, pieces[11].rotate(blokusAI::Rotation::Rot_270), { 0, blokusAI::Board::BoardSize - 3 });
    board.addPiece(blokusAI::Slot::P0, pieces[13], { 2, 2 });

    _viewer.setBoard(board);
}
