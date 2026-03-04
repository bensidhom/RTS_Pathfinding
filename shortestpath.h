#include "battlematrix.h"
#include <unordered_set>
#include <unordered_map>
#include <optional>

/**
 * @brief searchStartAndTargetPos search every start and target position
 * @param bM BattleMatrix
 * @return tuple composed of a vector of start position indexes and a vector of target position indexes
 */
std::tuple<std::vector<uint64_t>, std::vector<uint64_t>> searchStartAndTargetPos(const BattleMatrix& bM);

/**
 * @brief bfsShortestPathsSingleStartPos Breadth-first search algorithm to compute shortest path from a single start position to multiple targets.
 * @param bM BattleMatrix
 * @param startPosCount number of start position inside the BattleMatrix
 * @param startPosIndex start position index
 * @param occupiedPerDepth position indexes used by others units per depth
 * @param targetsPosReached [out] targets position reached (index, path length)
 * @param matrixPathTracking [out] matrix storing previous index visited for each position
 */
void bfsShortestPathsSingleStartPos(const BattleMatrix& bM, uint64_t startPosCount, uint64_t startPosIndex,
    const std::vector<std::unordered_set<uint64_t>>& occupiedPerDepth,
    std::vector<std::pair<uint64_t, size_t>>& targetsPosReached,
    std::vector<uint64_t>& matrixPathTracking);

/**
 * @brief backTrackingPathsSingleStartPos retrieve shortest paths generated from matrixPathTracking
 * @param targetsPosReached targets position reached (index, path length)
 * @param matrixPathTracking matrix storing previous index visited for each position
 * @param occupiedPerDepth [out] add position indexes used by current units per depth
 * @param targetsToPaths [out] store generated path per targets
 */
void backTrackingPathsSingleStartPos(const std::vector<std::pair<uint64_t, size_t>>& targetsPosReached,
    const std::vector<uint64_t>& matrixPathTracking,
    std::vector<std::unordered_set<uint64_t>>& occupiedPerDepth,
    std::unordered_map<uint64_t, std::vector<std::vector<uint64_t>>>& targetsToPaths);

/**
 * @brief shortestPathsMultipleStartPos compute every shortest paths possible given a vector of start and a vector of targets
 * @param bM BattleMatrix
 * @param startPosIndexes start position indexes
 * @param targetsToPaths [out] store generated path per targets
 * @return true if every existing targets have been reached, false otherwise
 */
bool shortestPathsMultipleStartPos(const BattleMatrix& bM, const std::vector<uint64_t>& startPosIndexes,
    std::unordered_map<uint64_t, std::vector<std::vector<uint64_t>>>& targetsToPaths);

/**
 * @brief selectShortestPaths select shortest paths containing each start and target only once.
 * Selection is done by first computing minimum path length by targets and then sorting targets by decreasing values.
 * Then, for each target, the minimum path length starting from a non selected start position is selected.
 * @param targetsToPaths generated path per targets
 * @return shortest paths if a solution was found, false otherwise
 */
std::optional<std::vector<std::vector<uint64_t>>> selectShortestPaths(
    const std::unordered_map<uint64_t, std::vector<std::vector<uint64_t>>>& targetsToPaths);

