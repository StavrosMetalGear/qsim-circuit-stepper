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


static void dump_statevector(std::shared_ptr<IBackendEx> backend) {
    auto sv = std::dynamic_pointer_cast<IStatevectorBackend>(backend);
    if (!sv) {
        std::cout << "--dump-state is only available for --backend statevector\n";
        return;
    }

    const auto& amps = sv->amplitudes_ref();
    const std::size_t dim = amps.size();

    if (dim > 32) {
        std::cout << "Statevector dim=" << dim << " (too large to print; limit is 32)\n";
        return;
    }

    std::cout << "Statevector amplitudes (index: re + i im):\n";
    for (std::size_t i = 0; i < dim; ++i) {
        std::cout << i << ": " << amps[i].real() << " + i" << amps[i].imag() << "\n";
    }
}

static void dump_reduced_rho(std::shared_ptr<IBackendEx> backend, std::uint32_t q) {
    if (!backend) return;
    if (q >= backend->num_qubits()) {
        std::cout << "Invalid qubit index for --dump-rho-qubit\n";
        return;
    }

    const auto r = backend->reduced_density_1q(q);
    std::cout << "Reduced density matrix rho(q" << q << ") =\n";
    std::cout << "[[" << r[0][0] << ", " << r[0][1] << "],\n";
    std::cout << " [" << r[1][0] << ", " << r[1][1] << "]]\n";
}
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
    default: return "UNKNOWN";
    }
}

static void print_circuit(const Circuit& c) {
    for (std::size_t i = 0; i < c.size(); ++i) {
        const auto ins = c[i];
        std::cout << i << ": " << op_name(ins.type) << " ";
        std::cout << "targets=[";
        for (std::size_t k = 0; k < ins.targets.size(); ++k) {
            std::cout << ins.targets[k] << (k+1<ins.targets.size()? ",":"");
        }
        std::cout << "]";
        if (!ins.params.empty()) {
            std::cout << " params=[";
            for (std::size_t k = 0; k < ins.params.size(); ++k) {
                std::cout << ins.params[k] << (k+1<ins.params.size()? ",":"");
            }
            std::cout << "]";
        }
        std::cout << "\n";
    }
}
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

if (opt.dump_state) {
    dump_statevector(backend);
}
if (opt.dump_rho) {
    dump_reduced_rho(backend, opt.dump_rho_qubit);
}

std::cout << "Finished.\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
