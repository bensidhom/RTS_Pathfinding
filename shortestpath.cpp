#include "shortestpath.h"
#include <numeric>
#include <limits>
#include <algorithm>
#include <iostream>

std::tuple<std::vector<uint64_t>, std::vector<uint64_t>> searchStartAndTargetPos(const BattleMatrix& bM)
{
    std::vector<uint64_t> startPos;
    std::vector<uint64_t> targetPos;
    for (uint64_t i = 0; i < bM.getPositionsCount(); ++i)
    {
        switch (bM.getPosition(i))
        {
        case BattleMatrix::Start:
            startPos.push_back(i);
            break;
        case BattleMatrix::Target:
            targetPos.push_back(i);
            break;
        default:
            break;
        }
    }
    return { startPos, targetPos };
}

void bfsShortestPathsSingleStartPos(const BattleMatrix& bM, uint64_t startPosCount, uint64_t startPosIndex,
    const std::vector<std::unordered_set<uint64_t>>& occupiedPerDepth,
    std::vector<std::pair<uint64_t, size_t>>& targetsPosReached,
    std::vector<uint64_t>& matrixPathTracking)
{
    std::vector<uint64_t> toBeVisitedIndexes = { startPosIndex };
    matrixPathTracking = std::vector<uint64_t>(bM.getPositionsCount());
    std::iota(matrixPathTracking.begin(), matrixPathTracking.end(), 0);
    size_t depth = 1;
    while (targetsPosReached.size() < startPosCount && !toBeVisitedIndexes.empty()) // bfs recursion
    {
        std::vector<uint64_t> nextDepthIndexesToBeVisited;
        for (uint64_t currIndex : toBeVisitedIndexes)
        {
            auto neighboursIndex = bM.findNeighboursIndex(currIndex);
            for (uint64_t neighbourIndex : neighboursIndex)
            {
                // check if already visited and not already occupied at this depth
                if (matrixPathTracking[neighbourIndex] == neighbourIndex &&
                    (occupiedPerDepth.size() <= depth || occupiedPerDepth[depth].count(neighbourIndex) == 0))
                {
                    auto neighbourPosition = bM.getPosition(neighbourIndex);
                    switch (neighbourPosition)
                    {
                    case BattleMatrix::Free:
                        nextDepthIndexesToBeVisited.push_back(neighbourIndex);
                        matrixPathTracking[neighbourIndex] = currIndex;
                        break;
                    case BattleMatrix::Start:
                        if (neighbourIndex != startPosIndex)
                        {
                            nextDepthIndexesToBeVisited.push_back(neighbourIndex);
                            matrixPathTracking[neighbourIndex] = currIndex;
                        }
                        break;
                    case BattleMatrix::Target:
                        nextDepthIndexesToBeVisited.push_back(neighbourIndex);
                        matrixPathTracking[neighbourIndex] = currIndex;
                        targetsPosReached.push_back({ neighbourIndex, depth + 1 });
                        break;
                    case BattleMatrix::Elevated:
                        break;
                    }
                }
            }
        }
        toBeVisitedIndexes = nextDepthIndexesToBeVisited;
        depth++; //  BREAKPOINT HERE
    }
}

void backTrackingPathsSingleStartPos(const std::vector<std::pair<uint64_t, size_t>>& targetsPosReached,
    const std::vector<uint64_t>& matrixPathTracking,
    std::vector<std::unordered_set<uint64_t>>& occupiedPerDepth,
    std::unordered_map<uint64_t, std::vector<std::vector<uint64_t>>>& targetsToPaths)
{
    for (size_t i = 0; i < targetsPosReached.size(); ++i)
    {
        uint64_t currIndex = targetsPosReached[i].first;
        std::vector<uint64_t> path(targetsPosReached[i].second); // ← path is created here
        occupiedPerDepth.resize(std::max(path.size(), occupiedPerDepth.size()));
        for (size_t j = 0; j < path.size(); ++j)
        {
            path[path.size() - 1 - j] = currIndex;
            occupiedPerDepth[path.size() - 1 - j].insert(currIndex);
            currIndex = matrixPathTracking[currIndex];
        }
        if (targetsToPaths.count(targetsPosReached[i].first) == 0)
        {
            targetsToPaths.insert({ targetsPosReached[i].first, {} });
        }
        targetsToPaths[targetsPosReached[i].first].push_back(path);
    }
}

bool shortestPathsMultipleStartPos(const BattleMatrix& bM, const std::vector<uint64_t>& startPosIndexes,
    std::unordered_map<uint64_t, std::vector<std::vector<uint64_t>>>& targetsToPaths)
{
    std::vector<std::unordered_set<uint64_t>> occupiedPerDepth;
    targetsToPaths.clear();

    std::cout << "  [Multi-Start BFS] Processing " << startPosIndexes.size() << " starts...\n";

    for (uint64_t startPosIndex : startPosIndexes)
    {
        auto [col, row] = bM.convertIndexToColRow(startPosIndex);
        std::cout << "    Start at (" << col << "," << row << ") → ";

        std::vector<std::pair<uint64_t, size_t>> targetsPosReached;
        std::vector<uint64_t> matrixPathTracking;

        bfsShortestPathsSingleStartPos(bM, startPosIndexes.size(), startPosIndex,
            occupiedPerDepth, targetsPosReached, matrixPathTracking);

        if (targetsPosReached.empty())
        {
            std::cout << "cannot reach any target (blocked by elevated star)\n";
            continue;                    // <--- THIS IS THE KEY FIX
        }

        std::cout << "reached " << targetsPosReached.size() << " target(s)\n";

        backTrackingPathsSingleStartPos(targetsPosReached, matrixPathTracking,
            occupiedPerDepth, targetsToPaths);
    }

    // We only need enough unique targets for the number of starts that actually moved
    if (targetsToPaths.size() < startPosIndexes.size())
    {
        std::cout << "  → FAILED: Not enough reachable targets ("
            << targetsToPaths.size() << "/" << startPosIndexes.size() << ")\n";
        return false;
    }

    std::cout << "  → All starts processed successfully\n";
    return true;
}

std::optional<std::vector<std::vector<uint64_t>>> selectShortestPaths(
    const std::unordered_map<uint64_t, std::vector<std::vector<uint64_t>>>& targetsToPaths)
{
    if (targetsToPaths.empty())
        return {};

    // Count unique starts that actually reached something
    std::unordered_set<uint64_t> uniqueStarts;
    for (const auto& [target, paths] : targetsToPaths) {
        for (const auto& path : paths) {
            if (!path.empty()) {
                uniqueStarts.insert(path.front());
            }
        }
    }

    // SPECIAL CASE: Single battle unit reaching multiple targets (map3)
    if (uniqueStarts.size() == 1) {
        std::cout << "  [Single-Unit Mode] One start inside star reaching "
            << targetsToPaths.size() << " targets → picking shortest path\n";

        std::vector<uint64_t> bestPath;
        size_t bestLength = std::numeric_limits<size_t>::max();

        for (const auto& [target, paths] : targetsToPaths) {
            for (const auto& path : paths) {
                if (!path.empty() && path.size() < bestLength) {
                    bestLength = path.size();
                    bestPath = path;
                }
            }
        }
        if (!bestPath.empty()) {
            return { {bestPath} };   // return exactly one path
        }
        return {};
    }

    // NORMAL CASE: Multiple distinct starts → original greedy assignment
    using TargetLength = std::pair<uint64_t, size_t>;
    std::vector<TargetLength> targetsLength;
    for (const auto& [target, paths] : targetsToPaths) {
        size_t minDepth = std::numeric_limits<size_t>::max();
        for (const auto& path : paths) {
            minDepth = std::min(minDepth, path.size());
        }
        targetsLength.push_back({ target, minDepth });
    }

    std::sort(targetsLength.begin(), targetsLength.end(),
        [](const TargetLength& left, const TargetLength& right) {
            return left.second > right.second;
        });

    std::vector<std::vector<uint64_t>> shortestPaths;
    std::unordered_set<uint64_t> startSelected;

    for (auto& [target, length] : targetsLength) {
        const auto& paths = targetsToPaths.at(target);
        size_t selectedMinDepth = std::numeric_limits<size_t>::max();
        size_t selectedInd = 0;
        bool found = false;

        for (size_t i = 0; i < paths.size(); ++i) {
            if (paths[i].size() < selectedMinDepth &&
                startSelected.count(paths[i].front()) == 0) {
                selectedMinDepth = paths[i].size();
                selectedInd = i;
                found = true;
            }
        }
        if (!found) {
            return {};
        }
        shortestPaths.push_back(paths[selectedInd]);
        startSelected.insert(paths[selectedInd].front());
    }
    return shortestPaths;
}
