#include "backend/StatevectorBackend.hpp"
#include <stdexcept>

StatevectorBackend::StatevectorBackend(int num_qubits)
    : m_numQubits(num_qubits), m_state() // adjust if your Statevector needs constructor args
{
    if (m_numQubits <= 0) throw std::invalid_argument("num_qubits must be > 0");

    // TODO: initialize |0...0> properly using your Repo #1 API
    // Example possibilities:
    // m_state = Statevector(m_numQubits);
    // m_state.set_zero_state();
}

void StatevectorBackend::apply(const Instruction& instr) {
    // For now: 1-qubit only stepping (enough to reach Bloch sphere soon)
    if (m_numQubits != 1) {
        throw std::runtime_error("StatevectorBackend: only 1-qubit supported currently");
    }

    if (instr.targets.empty()) {
        throw std::runtime_error("Instruction has no targets");
    }

    const int q = instr.targets[0];
    const double p = instr.params.empty() ? 0.0 : instr.params[0];

    switch (instr.type) {
    case OpType::H:
    case OpType::X:
    case OpType::Y:
    case OpType::Z:
    case OpType::RX:
    case OpType::RY:
    case OpType::RZ:
        apply_1q_gate(instr.type, q, p);
        break;

    case OpType::MEASURE:
        // TODO later: measurement sampling. For now do nothing.
        break;

    default:
        throw std::runtime_error("Unsupported op in 1-qubit backend right now");
    }
}

void StatevectorBackend::apply_1q_gate(OpType type, int /*target*/, double /*param*/) {
    // IMPORTANT: replace the bodies below with YOUR Repo #1 functions/methods.

    switch (type) {
    case OpType::H:
        // Example: m_state.apply_h();
        // or: apply_h(m_state);
        break;

    case OpType::X:
        // Example: m_state.apply_x();
        break;

    case OpType::Y:
        // Example: m_state.apply_y();
        break;

    case OpType::Z:
        // Example: m_state.apply_z();
        break;

    case OpType::RX:
        // Example: m_state.apply_rx(param);
        break;

    case OpType::RY:
        // Example: m_state.apply_ry(param);
        break;

    case OpType::RZ:
        // Example: m_state.apply_rz(param);
        break;

    default:
        throw std::runtime_error("apply_1q_gate: unsupported");
    }
}
