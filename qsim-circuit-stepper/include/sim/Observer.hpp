#pragma once
#include "circuit/Instruction.hpp"
#include <cstddef>

class Observer {
public:
    virtual ~Observer() = default;

    virtual void before_step(std::size_t step, const Instruction& instr) {}
    virtual void after_step(std::size_t step, const Instruction& instr) {}
};
