#ifndef MODEL_H
#define MODEL_H
#include <optional>
#include "battlematrix.h"
#include <string>

/**
 * @brief The Model class representing the available processing functions.
 */
class Model
{
public:
    Model();

    /**
     * @brief loadFromJson read, extract and load json data to the model.
     * @param jsonPath json file path
     * Json file must have at least following structure :
     *      {"layers" : [ { "data" : [battleMatrixData] } ] }
     * With battleMatrixData containing integers of size = n * n where n is an integer.
     * Integers representation :
     *      0 = Start position
     *      4 = Elevated position
     *      6 = Target position
     *      Else = Free position
     * Note that the same number of start and target position is required.
     * @param jsonPath json file path
     * @return BattleMatrix if structure is correct
     */
    std::optional<BattleMatrix> loadFromJson(const std::string& jsonPath);

    /**
     * @brief moveStartsToTargets find shortest path from starts to targets in order to minimize the maximum path length.
     * Every start can be used only once and every target must be reached only once.
     * @param bMOut [out] BattleMatrix used
     * @param shortestPaths [out] vector of shortest path found
     * @return true if a solution was found, false otherwise
     */
    bool moveStartsToTargets(BattleMatrix& bMOut, std::vector<std::vector<uint64_t>>& shortestPaths);

private:
    BattleMatrix bM;
    std::vector<uint64_t> startPosIndexes;
};

#endif // MODEL_H

