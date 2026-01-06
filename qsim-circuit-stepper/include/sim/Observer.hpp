// Observer.hpp
#pragma once
#include "Instruction.hpp"

class Observer {
public:
    virtual ~Observer() = default;

    virtual void before_step(
        size_t step,
        const Instruction& instr
    ) {
    }

    virtual void after_step(
        size_t step,
        const Instruction& instr
    ) {
    }
};

