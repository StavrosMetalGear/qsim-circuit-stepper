#include "backend/StatevectorBackend.hpp"

StatevectorBackend::StatevectorBackend(std::uint32_t num_qubits, std::size_t threads)
    : m_numQubits(num_qubits),
      m_threads(threads),
      m_state(num_qubits)
{
    m_state.set_zero_state();
}

void StatevectorBackend::apply(const Instruction& instr) {
    if (instr.targets.empty())
        throw std::runtime_error("Instruction has no targets");

    const std::uint32_t t = static_cast<std::uint32_t>(instr.targets[0]);

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

    case OpType::MEASURE:
        (void)m_state.measure_qubit(t, m_rng);
        break;

    default:
        throw std::runtime_error("Gate not implemented in backend yet");
    }
}
