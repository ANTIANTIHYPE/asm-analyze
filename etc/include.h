// Include libraries and define functions to use them in main.cpp

#pragma once

#include "dbg.hpp"

#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <ctime>

// function prototypes (smallest->biggest)
[[nodiscard]] auto isInstruction(const std::string& opcode) -> bool;
[[nodiscard]] auto trim(const std::string& str) -> std::string;
[[nodiscard]] auto isDirective(const std::string& opcode) -> bool;
[[nodiscard]] auto getOperand(const std::string& line) -> std::string;
[[nodiscard]] auto analyzeLine(const std::string& line) -> std::string;
[[nodiscard]] auto analyzeOperands(const std::string& operands) -> std::string;
[[nodiscard]] auto isMemoryAddressingMode(const std::string& operand) -> bool;
[[nodiscard]] auto getArchitecture(const std::string& filename) -> std::string;
[[nodiscard]] auto analyzeOperand(const std::string& operand, bool appendType = false) -> std::string;
[[nodiscard]] auto analyzeDirective(const std::string& opcode, const std::string& operand) -> std::string;
[[nodiscard]] auto analyzeInstruction(const std::string& opcode, const std::string& operands, const std::string& line) -> std::string;