
#ifndef BATTLEMATRIX_H
#define BATTLEMATRIX_H

#include <cstdint>
#include <vector>
#include <tuple>

/**
 * @brief The BattleMatrix class represent a square 2D battle matrix.
 */
class BattleMatrix
{
public:
    /**
     * @brief The Position enum represent the value of a position inside the 2D grid
     */
    enum Position
    {
        Free = 0,
        Start = 1,
        Target = 2,
        Elevated = 3
    };

    /**
     * @brief BattleMatrix constructor to initialize the battle matrix
     * @param data containing the square battle matrix data
     * @param size size of the square
     */
    BattleMatrix(const std::vector<Position>& data = {}, uint64_t size = 0);

    /**
     * @brief getSize getter of the size of the square of the battle matrix
     * @return size of the square
     */
    uint64_t getSize() const;

    /**
     * @brief getPositionsCount getter of the total number of position inside the battle matrix
     * @return total number of position
     */
    uint64_t getPositionsCount() const;

    /**
     * @brief getPosition getter of a single Position inside the battle matrix
     * @param index 1D vector index
     * @return Position at index
     */
    Position getPosition(uint64_t index) const;

    /**
     * @brief convertIndexToColRow convert 1D vector index to 2D column-row indices
     * @param index 1D vector index
     * @return 2D column-row indices
     */
    std::tuple<uint64_t, uint64_t> convertIndexToColRow(uint64_t index) const;

    /**
     * @brief findNeighboursIndex find neighbours
     * @param index
     * @return
     */
    std::vector<uint64_t> findNeighboursIndex(uint64_t index) const;

private:
    std::vector<Position> data; // containing the square battle matrix data
    uint64_t size; // size of the square matrix
};

#endif // BATTLEMATRIX_H
