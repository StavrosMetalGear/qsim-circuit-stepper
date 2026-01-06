// Instruction.hpp
#pragma once
#include <vector>

enum class OpType {
    H, X, Y, Z,
    RX, RY, RZ,
    CNOT,
    MEASURE
};

struct Instruction {
    OpType type;
    std::vector<int> targets;
    std::vector<double> params;
};
