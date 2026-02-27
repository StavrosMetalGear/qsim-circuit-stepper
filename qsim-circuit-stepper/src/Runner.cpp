#include "sim/Runner.hpp"

Runner::Runner(Stepper& stepper) : s(stepper) {}

bool Runner::should_break(std::size_t pc, const Instruction& instr,
                          const Breakpoints& bp, StopReason& out_reason) const {
    if (bp.step_indices.find(pc) != bp.step_indices.end()) {
        out_reason = StopReason::BreakpointStepIndex;
        return true;
    }
    if (bp.op_types.find(instr.type) != bp.op_types.end()) {
        out_reason = StopReason::BreakpointOpType;
        return true;
    }
    return false;
}

RunResult Runner::run(const Breakpoints& bp) {
    while (!s.done()) {
        const std::size_t pc = s.current_pc();
        const Instruction instr = s.peek();

        StopReason reason{};
        if (should_break(pc, instr, bp, reason)) {
            return { reason, pc, instr };
        }
        s.step();
    }
    return { StopReason::Finished, s.current_pc(), Instruction{OpType::H, {}, {}} };
}

RunResult Runner::run_until_pc(std::size_t target_pc, const Breakpoints& bp) {
    while (!s.done()) {
        const std::size_t pc = s.current_pc();
        const Instruction instr = s.peek();

        if (pc >= target_pc) {
            return { StopReason::BreakpointStepIndex, pc, instr };
        }

        StopReason reason{};
        if (should_break(pc, instr, bp, reason)) {
            return { reason, pc, instr };
        }
        s.step();
    }
    return { StopReason::Finished, s.current_pc(), Instruction{OpType::H, {}, {}} };
}
