#include "battlematrix.h"

BattleMatrix::BattleMatrix(const std::vector<Position>& data, uint64_t size)
    : data(data), size(size)
{
}

uint64_t BattleMatrix::getSize() const
{
    return size;
}

uint64_t BattleMatrix::getPositionsCount() const
{
    return data.size();
}

BattleMatrix::Position BattleMatrix::getPosition(uint64_t index) const
{
    return data[index];
}

std::tuple<uint64_t, uint64_t> BattleMatrix::convertIndexToColRow(uint64_t index) const
{
    return { index % size, index / size };
}

std::vector<uint64_t> BattleMatrix::findNeighboursIndex(uint64_t index) const
{
    auto [col, row] = convertIndexToColRow(index);
    std::vector<uint64_t> neighboursIndex;
    if (row > 0)
    {
        neighboursIndex.push_back(index - size); // top
    }
    if (col + 1 < size)
    {
        neighboursIndex.push_back(index + 1); // right
    }
    if (row + 1 < size)
    {
        neighboursIndex.push_back(index + size); // bottom
    }
    if (col > 0)
    {
        neighboursIndex.push_back(index - 1); // left
    }
    return neighboursIndex;
}
