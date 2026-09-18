#include "TetrisGame.hpp"

#include <algorithm>
#include <iostream>

namespace
{
struct ShapeCell
{
    int32_t x;
    int32_t y;
};

using ShapeRotation = std::array<ShapeCell, 4>;
using ShapeSet = std::array<ShapeRotation, 4>;

constexpr ShapeSet kShapeI = {{
    {{{0, 1}, {1, 1}, {2, 1}, {3, 1}}},
    {{{2, 0}, {2, 1}, {2, 2}, {2, 3}}},
    {{{0, 2}, {1, 2}, {2, 2}, {3, 2}}},
    {{{1, 0}, {1, 1}, {1, 2}, {1, 3}}},
}};

constexpr ShapeSet kShapeO = {{
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}},
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}},
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}},
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}},
}};

constexpr ShapeSet kShapeT = {{
    {{{1, 0}, {0, 1}, {1, 1}, {2, 1}}},
    {{{1, 0}, {1, 1}, {2, 1}, {1, 2}}},
    {{{0, 1}, {1, 1}, {2, 1}, {1, 2}}},
    {{{1, 0}, {0, 1}, {1, 1}, {1, 2}}},
}};

constexpr ShapeSet kShapeL = {{
    {{{2, 0}, {0, 1}, {1, 1}, {2, 1}}},
    {{{1, 0}, {1, 1}, {1, 2}, {2, 2}}},
    {{{0, 1}, {1, 1}, {2, 1}, {0, 2}}},
    {{{0, 0}, {1, 0}, {1, 1}, {1, 2}}},
}};

constexpr ShapeSet kShapeJ = {{
    {{{0, 0}, {0, 1}, {1, 1}, {2, 1}}},
    {{{1, 0}, {2, 0}, {1, 1}, {1, 2}}},
    {{{0, 1}, {1, 1}, {2, 1}, {2, 2}}},
    {{{1, 0}, {1, 1}, {0, 2}, {1, 2}}},
}};

constexpr ShapeSet kShapeS = {{
    {{{1, 0}, {2, 0}, {0, 1}, {1, 1}}},
    {{{1, 0}, {1, 1}, {2, 1}, {2, 2}}},
    {{{1, 1}, {2, 1}, {0, 2}, {1, 2}}},
    {{{0, 0}, {0, 1}, {1, 1}, {1, 2}}},
}};

constexpr ShapeSet kShapeZ = {{
    {{{0, 0}, {1, 0}, {1, 1}, {2, 1}}},
    {{{2, 0}, {1, 1}, {2, 1}, {1, 2}}},
    {{{0, 1}, {1, 1}, {1, 2}, {2, 2}}},
    {{{1, 0}, {0, 1}, {1, 1}, {0, 2}}},
}};

constexpr const ShapeSet& ShapeFor(TetrisPieceType type)
{
    switch (type)
    {
        case TetrisPieceType::I:
            return kShapeI;
        case TetrisPieceType::O:
            return kShapeO;
        case TetrisPieceType::T:
            return kShapeT;
        case TetrisPieceType::L:
            return kShapeL;
        case TetrisPieceType::J:
            return kShapeJ;
        case TetrisPieceType::S:
            return kShapeS;
        case TetrisPieceType::Z:
            return kShapeZ;
    }

    return kShapeI;
}
} // namespace

TetrisGame::TetrisGame()
{
    Reset();
    Score();
    std::cout << "Score is used!" << std::endl;
}

void TetrisGame::Reset()
{
    _board.fill(0);
    _boardWidth = BaseBoardWidth;
    _nextSpeedThreshold  = 100;
    _nextExpandThreshold = 1000;

    _pieceSequence = {
        TetrisPieceType::I, TetrisPieceType::O, TetrisPieceType::T, TetrisPieceType::L,
        TetrisPieceType::J, TetrisPieceType::S, TetrisPieceType::Z,
    };
    _nextSequenceIndex = 0;
    _score = 0;
    _linesCleared = 0;
    _gameOver = false;
    SpawnNextPiece();
}

bool TetrisGame::AdvanceGravityStep()
{
    if (_gameOver)
    {
        return false;
    }

    TetrisActivePiece nextPiece = _activePiece;
    nextPiece.y += 1;
    if (!Collides(nextPiece))
    {
        _activePiece = nextPiece;
        return false;
    }

    LockActivePiece();
    return true;
}

bool TetrisGame::SoftDrop()
{
    return AdvanceGravityStep();
}

void TetrisGame::MoveLeft()
{
    if (_gameOver)
    {
        return;
    }

    TetrisActivePiece nextPiece = _activePiece;
    nextPiece.x -= 1;
    if (!Collides(nextPiece))
    {
        _activePiece = nextPiece;
    }
}

void TetrisGame::MoveRight()
{
    if (_gameOver)
    {
        return;
    }

    TetrisActivePiece nextPiece = _activePiece;
    nextPiece.x += 1;
    if (!Collides(nextPiece))
    {
        _activePiece = nextPiece;
    }
}

void TetrisGame::RotateClockwise()
{
    if (_gameOver)
    {
        return;
    }

    TetrisActivePiece nextPiece = _activePiece;
    nextPiece.rotation = (nextPiece.rotation + 1) % 4;
    if (!Collides(nextPiece))
    {
        _activePiece = nextPiece;
    }
}

bool TetrisGame::IsCellOccupied(int32_t x, int32_t y) const
{
    return _board[CellIndex(x, y)] != 0;
}

bool TetrisGame::IsGameOver() const
{
    return _gameOver;
}

int32_t TetrisGame::Score() const
{
    return _score;
}

int32_t TetrisGame::LinesCleared() const
{
    return _linesCleared;
}

const TetrisActivePiece& TetrisGame::ActivePiece() const
{
    return _activePiece;
}

TetrisPieceType TetrisGame::NextPieceType() const
{
    return _pieceSequence[_nextSequenceIndex % _pieceSequence.size()];
}

std::array<TetrisCellPosition, 4> TetrisGame::ActivePieceBlocks() const
{
    std::array<TetrisCellPosition, 4> blocks{};
    const ShapeRotation& rotation = ShapeFor(_activePiece.type)[_activePiece.rotation];

    for (std::size_t i = 0; i < rotation.size(); ++i)
    {
        blocks[i] = {
            _activePiece.x + rotation[i].x,
            _activePiece.y + rotation[i].y,
        };
    }

    return blocks;
}

void TetrisGame::ClearBoard()
{
    _board.fill(0);
    _score = 0;
    _linesCleared = 0;
    _gameOver = false;
}

void TetrisGame::SetOccupied(int32_t x, int32_t y, bool occupied)
{
    _board[CellIndex(x, y)] = occupied ? 1 : 0;
}

void TetrisGame::ForceActivePiece(TetrisPieceType type, int32_t rotation, int32_t x, int32_t y)
{
    _activePiece = {
        type,
        rotation % 4,
        x,
        y,
    };
}

std::size_t TetrisGame::CellIndex(int32_t x, int32_t y) const
{
    return static_cast<std::size_t>(y * _boardWidth + x);
}

bool TetrisGame::Collides(const TetrisActivePiece& piece) const
{
    const ShapeRotation& rotation = ShapeFor(piece.type)[piece.rotation];
    for (const ShapeCell& cell : rotation)
    {
        const int32_t cellX = piece.x + cell.x;
        const int32_t cellY = piece.y + cell.y;
        if (cellX < 0 || cellX >= _boardWidth || cellY < 0 || cellY >= BoardHeight)
        {
            return true;
        }

        if (IsCellOccupied(cellX, cellY))
        {
            return true;
        }
    }

    return false;
}

void TetrisGame::LockActivePiece()
{
    const ShapeRotation& rotation = ShapeFor(_activePiece.type)[_activePiece.rotation];
    for (const ShapeCell& cell : rotation)
    {
        _board[CellIndex(_activePiece.x + cell.x, _activePiece.y + cell.y)] = 1;
    }

    ClearCompletedLines();
    SpawnNextPiece();
}

void TetrisGame::ClearCompletedLines()
{
    int32_t clearedThisLock = 0;
    for (int32_t y = BoardHeight - 1; y >= 0; --y)
    {
        bool rowComplete = true;
        for (int32_t x = 0; x < _boardWidth; ++x)
        {
            if (!IsCellOccupied(x, y))
            {
                rowComplete = false;
                break;
            }
        }

        if (!rowComplete)
        {
            continue;
        }

        ++clearedThisLock;
        for (int32_t shiftY = y; shiftY > 0; --shiftY)
        {
            for (int32_t x = 0; x < _boardWidth; ++x)
            {
                _board[CellIndex(x, shiftY)] = _board[CellIndex(x, shiftY - 1)];
            }
        }

        for (int32_t x = 0; x < _boardWidth; ++x)
        {
            _board[CellIndex(x, 0)] = 0;
        }

        ++y;
    }

    _linesCleared += clearedThisLock;
    _score += clearedThisLock * 100;
}

void TetrisGame::SpawnNextPiece()
{
    const int32_t spawnX = _boardWidth / 2 - 1;
    _activePiece = {
        _pieceSequence[_nextSequenceIndex % _pieceSequence.size()],
        0,
        spawnX,
        0,
    };
    ++_nextSequenceIndex;
    _gameOver = Collides(_activePiece);
}

void TetrisGame::ApplyScoreThresholds()
{
    // Ускорение каждые 100 очков (порог растёт)
    while (_score >= _nextSpeedThreshold)
    {
        _nextSpeedThreshold += 100;
        // gravityStep пересчитывается в TetrisApplication по Score()
        // здесь только фиксируем, что порог пройден (см. GetGravityStepSeconds)
    }

    // Расширение поля каждые 1000 очков (порог растёт)
    while (_score >= _nextExpandThreshold && _boardWidth < MaxBoardWidth)
    {
        _nextExpandThreshold += 1000;
        ++_boardWidth;

        // Сдвигаем содержимое: добавляем столбец слева и справа,
        // сохраняя существующие блоки по центру.
        BoardCells newBoard {};
        const int32_t shift = 1; // на сколько столбцов расширили
        for (int32_t y = 0; y < BoardHeight; ++y)
        {
            for (int32_t x = 0; x < _boardWidth - shift * 2; ++x)
            {
                newBoard[static_cast<std::size_t>(y * _boardWidth + x + shift)] =
                    _board[static_cast<std::size_t>(y * (_boardWidth - shift * 2) + x)];
            }
        }
        _board = newBoard;

        // Сдвигаем активную фигуру вправо
        _activePiece.x += shift;
    }
}

float TetrisGame::GetGravityStepSeconds() const
{
    // Базовый шаг 0.10 c, каждые 100 очков — на 10% быстрее,
    // но не быстрее 0.02 c.
    const int32_t speedTier = _score / 100;
    float step = 0.50f;             
    for (int32_t i = 0; i < speedTier; ++i)
        step *= 0.9f;
    if (step < 0.02f)
        step = 0.02f;
    return step;
}

void TetrisGame::AddScore(int32_t amount)
{
    if (amount <= 0)
    {
        return;
    }

    _score += amount;
    ApplyScoreThresholds();
}
