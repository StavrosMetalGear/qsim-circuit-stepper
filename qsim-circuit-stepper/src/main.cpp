#include "circuit/Circuit.hpp"
#include "sim/stepper.hpp"
#include "sim/Runner.hpp"
#include "sim/ShotsRunner.hpp"

#include "sim/BlochObserver.hpp"
#include "sim/PhaseObserver.hpp"
#include "sim/MetricsObserver.hpp"
#include "sim/TraceObserver.hpp"

#include "sim/CliOptions.hpp"
#include "sim/CircuitParser.hpp"
#include "sim/QasmParser.hpp"

#include "backend/StatevectorBackend.hpp"
#include "backend/DensityMatrixBackend.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

static Circuit demo_bell() {
    Circuit c;
    c.add({ OpType::H,    {0},   {} });
    c.add({ OpType::CNOT, {0,1}, {} });
    c.add({ OpType::MEASURE, {0}, {} });
    c.add({ OpType::MEASURE, {1}, {} });
    return c;
}
static Circuit demo_rot1q() {
    Circuit c;
    c.add({ OpType::RX, {0}, {1.0} });
    c.add({ OpType::RZ, {0}, {0.7} });
    c.add({ OpType::RY, {0}, {1.2} });
    return c;
}

int main(int argc, char** argv) {
    try {
        CliOptions opt = parse_cli(argc, argv);

        Circuit c;
        if (!opt.qasm_path.empty()) {
            auto pq = QasmParser::parse_file(opt.qasm_path);
            c = pq.circuit;
            if (!opt.qubits_set) opt.qubits = pq.qubits;
        } else if (!opt.file_path.empty()) {
            auto pc = CircuitParser::parse_file(opt.file_path);
            c = pc.circuit;
            if (!opt.qubits_set) opt.qubits = pc.inferred_qubits;
        } else {
            c = (opt.demo == "rot1q") ? demo_rot1q() : demo_bell();
        }

        std::shared_ptr<IBackendEx> backend;

        if (opt.backend == "density") {
            backend = std::make_shared<DensityMatrixBackend>(opt.qubits, opt.depolarize, opt.dephase, opt.amp_damp);
        } else {
            backend = std::make_shared<StatevectorBackend>(opt.qubits, 1);
        }

        if (opt.has_seed) backend->set_seed(opt.seed);

        if (opt.shots > 0) {
            ShotsRunner sr(c, backend);
            auto hist = sr.run(opt.shots, opt.has_seed ? opt.seed : 0);
            std::cout << "shots=" << opt.shots << " seed=" << (opt.has_seed ? std::to_string(opt.seed) : "(none)") << "\n";
            for (auto& [k,v] : hist) std::cout << k << " : " << v << "\n";
            return 0;
        }

        Stepper stepper(c, backend);

        if (opt.enable_bloch) stepper.add_observer(std::make_shared<BlochObserver>(backend, opt.bloch_qubit));
        if (opt.enable_metrics) stepper.add_observer(std::make_shared<MetricsObserver>(backend, opt.metrics_qubit));

        // Phase is statevector-only; if user wants it, only attach when using statevector backend
        if (opt.enable_phase && opt.backend != "density") {
            auto sv = std::dynamic_pointer_cast<IStatevectorBackend>(backend);
            if (sv) stepper.add_observer(std::make_shared<PhaseObserver>(sv, opt.phase_i, opt.phase_j));
        }

        if (!opt.trace_path.empty()) {
            stepper.add_observer(std::make_shared<TraceObserver>(
                backend, opt.trace_path, opt.bloch_qubit, opt.phase_i, opt.phase_j
            ));
        }

        while (!stepper.done()) stepper.step();
        std::cout << "Finished.\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
