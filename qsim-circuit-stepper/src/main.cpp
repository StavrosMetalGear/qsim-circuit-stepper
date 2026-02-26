#include "circuit/Circuit.hpp"
#include "sim/stepper.hpp"
#include "backend/StatevectorBackend.hpp"

#include "sim/BlochObserver.hpp"
#include "sim/PhaseObserver.hpp"
#include "sim/Runner.hpp"

#include <iostream>
#include <memory>

static const char* op_name(OpType t) {
    switch (t) {
        case OpType::H: return "H";
        case OpType::X: return "X";
        case OpType::Y: return "Y";
        case OpType::Z: return "Z";
        case OpType::RX: return "RX";
        case OpType::RY: return "RY";
        case OpType::RZ: return "RZ";
        case OpType::CNOT: return "CNOT";
        case OpType::MEASURE: return "MEASURE";
        default: return "?";
    }
}

int main() {
    Circuit c;
    c.add({ OpType::H,    {0},   {} });
    c.add({ OpType::CNOT, {0,1}, {} });
    c.add({ OpType::MEASURE, {0}, {} });

    auto backend = std::make_shared<StatevectorBackend>(2, 1);
    Stepper stepper(c, backend);

    stepper.add_observer(std::make_shared<BlochObserver>(backend, 0));
    stepper.add_observer(std::make_shared<PhaseObserver>(backend, 0, 3));

    Runner runner(stepper);

    Breakpoints bp;
    bp.op_types.insert(OpType::MEASURE);     // stop BEFORE MEASURE

    // Run until breakpoint
    auto r = runner.run(bp);

    if (r.reason != StopReason::Finished) {
        std::cout << "Stopped at pc=" << r.pc << " before op " << op_name(r.next_instr.type) << "\n";
    } else {
        std::cout << "Finished.\n";
        return 0;
    }

    // Step once manually (execute MEASURE)
    std::cout << "Manual step...\n";
    stepper.step();

    // Continue to end
    auto r2 = runner.run(Breakpoints{});
    if (r2.reason == StopReason::Finished) {
        std::cout << "Finished after continue.\n";
    }
    return 0;
}
