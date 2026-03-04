#include "model.h"
#include "shortestpath.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <cctype>
#include <iostream>
Model::Model()
{
}

std::optional<BattleMatrix> Model::loadFromJson(const std::string& jsonPath)
{
    startPosIndexes.clear();

    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        std::cerr << "❌ Failed to open: " << jsonPath << std::endl;
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonStr = buffer.str();

    size_t dataPos = jsonStr.find("\"data\"");
    if (dataPos == std::string::npos) {
        std::cerr << "❌ No \"data\" array found in " << jsonPath << std::endl;
        return {};
    }

    size_t startBracket = jsonStr.find('[', dataPos);
    if (startBracket == std::string::npos) {
        std::cerr << "❌ No [ after data key in " << jsonPath << std::endl;
        return {};
    }

    // === ROBUST NUMBER EXTRACTION ===
    std::vector<BattleMatrix::Position> matrixData;
    size_t i = startBracket + 1;
    while (i < jsonStr.length() && matrixData.size() < 4096) {  // safety cap
        char c = jsonStr[i];

        if (std::isspace(static_cast<unsigned char>(c)) || c == ',') {
            ++i;
            continue;
        }

        if (c == ']') break;   // end of array

        if (std::isdigit(static_cast<unsigned char>(c)) || c == '-') {
            size_t numStart = i;
            ++i;
            while (i < jsonStr.length() && std::isdigit(static_cast<unsigned char>(jsonStr[i]))) ++i;

            std::string numStr = jsonStr.substr(numStart, i - numStart);
            try {
                int val = std::stoi(numStr);
                BattleMatrix::Position pos = BattleMatrix::Free;
                switch (val) {
                case 0:  pos = BattleMatrix::Start;    break;
                case 6:  pos = BattleMatrix::Target;   break;
                case 4:  pos = BattleMatrix::Elevated; break;
                default: pos = BattleMatrix::Free;     break;
                }
                matrixData.push_back(pos);
                if (val == 0) startPosIndexes.push_back(matrixData.size() - 1);
            }
            catch (...) {}
            continue;
        }
        ++i;
    }

    const uint64_t EXPECTED = 4096;  // 64x64
    if (matrixData.size() != EXPECTED) {
        std::cout << "   ⚠️  Parsed " << matrixData.size()
            << " numbers → auto-fixing to " << EXPECTED
            << " (64x64) by "
            << (matrixData.size() > EXPECTED ? "trimming extras" : "padding with -1 (Free)")
            << std::endl;

        if (matrixData.size() > EXPECTED) {
            matrixData.resize(EXPECTED);
            startPosIndexes.erase(
                std::remove_if(startPosIndexes.begin(), startPosIndexes.end(),
                    [EXPECTED](uint64_t idx) { return idx >= EXPECTED; }),
                startPosIndexes.end());
        }
        else {
            while (matrixData.size() < EXPECTED) {
                matrixData.push_back(BattleMatrix::Free);
            }
        }
    }

    uint64_t size = 64;
    bM = BattleMatrix(matrixData, size);

    std::cout << "✅ Loaded " << jsonPath
        << " → " << size << "x" << size
        << " grid | Starts: " << startPosIndexes.size()
        << " | Targets: " << std::count_if(matrixData.begin(), matrixData.end(),
            [](auto p) { return p == BattleMatrix::Target; })
        << std::endl;

    return bM;
}

bool Model::moveStartsToTargets(
    BattleMatrix& bMOut,
    std::vector<std::vector<uint64_t>>& shortestPaths)
{
    shortestPaths.clear();

    if (startPosIndexes.empty())
        return false;

    std::unordered_map<uint64_t,
        std::vector<std::vector<uint64_t>>> targetsToPaths;

    // IMPORTANT: use the internal bM loaded previously
    if (!shortestPathsMultipleStartPos(bM, startPosIndexes, targetsToPaths))
        return false;

    auto optShortestPaths = selectShortestPaths(targetsToPaths);

    if (!optShortestPaths)
        return false;

    bMOut = bM;
    shortestPaths = *optShortestPaths;

    return true;
}