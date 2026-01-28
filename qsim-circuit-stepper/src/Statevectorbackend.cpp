#include "backend/StatevectorBackend.hpp"
#include <stdexcept>

StatevectorBackend::StatevectorBackend(std::uint32_t num_qubits, std::size_t threads)
    : m_numQubits(num_qubits),
    m_threads(threads),
    m_state(num_qubits)
{
    m_state.set_zero_state();
}

void StatevectorBackend::apply(const Instruction& instr) {
    if (instr.type == OpType::CNOT) {
        if (instr.targets.size() != 2) throw std::runtime_error("CNOT needs 2 targets");
        m_state.CNOT((std::uint32_t)instr.targets[0], (std::uint32_t)instr.targets[1], m_threads);
        return;
    }

    if (instr.targets.size() != 1) throw std::runtime_error("Op needs 1 target");
    auto t = (std::uint32_t)instr.targets[0];

    switch (instr.type) {
    case OpType::H:
        m_state.H(t, m_threads);
        break;
    case OpType::X:
        m_state.X(t, m_threads);
        break;
    case OpType::Z:
        m_state.Z(t, m_threads);
        break;

    case OpType::MEASURE: {
        // (optional: store result somewhere later)
        (void)m_state.measure_qubit(t, m_rng);
        break;
    }

                        // Not implemented yet in your simulator:
    case OpType::Y:
    case OpType::RX:
    case OpType::RY:
    case OpType::RZ:
        throw std::runtime_error("Gate not implemented in tiny simulator yet (Y/RX/RY/RZ)");

    default:
        throw std::runtime_error("Unknown operation");
    }
}

