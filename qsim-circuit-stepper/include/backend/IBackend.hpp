#pragma once

#include "circuit/Instruction.hpp"

class IBackend {
public:
    virtual ~IBackend() = default;

    // Apply one instruction to the backend state
    virtual void apply(const Instruction& instr) = 0;
};
