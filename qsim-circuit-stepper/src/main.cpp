#include "circuit/Circuit.hpp"
#include "backend/StatevectorBackend.hpp"
#include "sim/stepper.hpp"
#include "sim/Runner.hpp"
#include "sim/ShotsRunner.hpp"

#include "sim/BlochObserver.hpp"
#include "sim/PhaseObserver.hpp"
#include "sim/TraceObserver.hpp"
#include "sim/MetricsObserver.hpp"
#include "sim/CliOptions.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// Parse op name for breakpoints
static bool parse_op(const std::string& s, OpType& out) {
    if (s == "H") { out = OpType::H; return true; }
    if (s == "X") { out = OpType::X; return true; }
    if (s == "Y") { out = OpType::Y; return true; }
    if (s == "Z") { out = OpType::Z; return true; }
    if (s == "RX") { out = OpType::RX; return true; }
    if (s == "RY") { out = OpType::RY; return true; }
    if (s == "RZ") { out = OpType::RZ; return true; }
    if (s == "CNOT") { out = OpType::CNOT; return true; }
    if (s == "MEASURE") { out = OpType::MEASURE; return true; }
    return false;
}

// Build demo circuits
static Circuit build_demo(const CliOptions& opt) {
    Circuit c;

    if (opt.demo == "bell") {
        c.add({ OpType::H,    {0},   {} });
        c.add({ OpType::CNOT, {0,1}, {} });
        c.add({ OpType::MEASURE, {0}, {} });
        c.add({ OpType::MEASURE, {1}, {} });
        return c;
    }

    if (opt.demo == "rot1q") {
        c.add({ OpType::RX, {0}, {1.0} });
        c.add({ OpType::RZ, {0}, {0.7} });
        c.add({ OpType::RY, {0}, {1.2} });
        return c;
    }

    throw std::runtime_error("Unknown demo: " + opt.demo);
}

int main(int argc, char** argv) {
    try {
        const CliOptions opt = parse_cli(argc, argv);

        Circuit c = build_demo(opt);
        auto backend = std::make_shared<StatevectorBackend>(opt.qubits, 1);

        if (opt.has_seed) backend->set_seed(opt.seed);

        // ---- SHOTS MODE ----
        if (opt.shots > 0) {
            ShotsRunner sr(c, backend);
            auto hist = sr.run(opt.shots, opt.has_seed ? opt.seed : 0);

            std::cout << "shots=" << opt.shots
                      << " seed=" << (opt.has_seed ? std::to_string(opt.seed) : std::string("(none)"))
                      << "\n";
            for (const auto& [bits, count] : hist) {
                std::cout << bits << " : " << count << "\n";
            }
            return 0;
        }

        // ---- STEP MODE ----
        Stepper stepper(c, backend);

        if (opt.enable_bloch) {
            stepper.add_observer(std::make_shared<BlochObserver>(backend, opt.bloch_qubit));
        }
        if (opt.enable_phase) {
            stepper.add_observer(std::make_shared<PhaseObserver>(backend, opt.phase_i, opt.phase_j));
        }
        if (opt.enable_metrics) {
            stepper.add_observer(std::make_shared<MetricsObserver>(backend, opt.metrics_qubit));
        }
        if (!opt.trace_path.empty()) {
            stepper.add_observer(std::make_shared<TraceObserver>(
                backend, opt.trace_path, opt.bloch_qubit, opt.phase_i, opt.phase_j
            ));
        }

        Runner runner(stepper);
        Breakpoints bp;

        if (opt.break_on_step) bp.step_indices.insert(opt.break_step);
        if (opt.break_on_op) {
            OpType t;
            if (!parse_op(opt.break_op, t)) throw std::runtime_error("Unknown op for --break-on: " + opt.break_op);
            bp.op_types.insert(t);
        }

        auto r = runner.run(bp);

        if (r.reason == StopReason::Finished) {
            std::cout << "Finished.\n";
            return 0;
        }

        std::cout << "Stopped at pc=" << r.pc << " (breakpoint)\n";
        std::cout << "Tip: call stepper.step() manually or run again.\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
