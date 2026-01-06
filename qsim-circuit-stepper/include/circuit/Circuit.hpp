// Circuit.hpp
#pragma once
#include "Instruction.hpp"
#include <vector>

class Circuit {
public:
    void add(const Instruction& instr) {
        instructions.push_back(instr);
    }

    const Instruction& operator[](size_t i) const {
        return instructions[i];
    }

    size_t size() const { return instructions.size(); }

private:
    std::vector<Instruction> instructions;
};
#pragma once
