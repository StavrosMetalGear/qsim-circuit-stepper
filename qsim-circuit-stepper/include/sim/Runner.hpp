#pragma once

#include "sim/stepper.hpp"
#include "circuit/Instruction.hpp"

#include <cstddef>
#include <unordered_set>

struct Breakpoints {
    // Stop BEFORE executing these step indices
    std::unordered_set<std::size_t> step_indices;

    // Stop BEFORE executing an instruction of these types
    std::unordered_set<OpType> op_types;
};

enum class StopReason {
    Finished,
    BreakpointStepIndex,
    BreakpointOpType
};

struct RunResult {
    StopReason reason;
    std::size_t pc;          // next instruction index
    Instruction next_instr;  // valid unless Finished
};

class Runner {
public:
    explicit Runner(Stepper& stepper);

    RunResult run(const Breakpoints& bp);
    RunResult run_until_pc(std::size_t target_pc, const Breakpoints& bp);

private:
    Stepper& s;

    bool should_break(std::size_t pc, const Instruction& instr,
                      const Breakpoints& bp, StopReason& out_reason) const;
};
