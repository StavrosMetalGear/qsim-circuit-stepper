#include "backend/StatevectorBackend.hpp"

StatevectorBackend::StatevectorBackend(std::uint32_t num_qubits, std::size_t threads)
    : m_numQubits(num_qubits),
      m_threads(threads),
      m_state(num_qubits),
      m_rng(0),
      m_lastMeas(num_qubits, -1)
{
    m_state.set_zero_state();
}

void StatevectorBackend::reset() {
    m_state.set_zero_state();
    std::fill(m_lastMeas.begin(), m_lastMeas.end(), -1);
}

void StatevectorBackend::set_seed(std::uint64_t seed) {
    m_rng.seed(seed);
}

std::vector<std::complex<double>> StatevectorBackend::amplitudes() const {
    const auto& amps = m_state.amplitudes();
    return std::vector<std::complex<double>>(amps.begin(), amps.end());
}

void StatevectorBackend::apply(const Instruction& instr) {
    if (instr.type == OpType::CNOT) {
        if (instr.targets.size() != 2) {
            throw std::runtime_error("CNOT requires 2 targets (control, target)");
        }
        const std::uint32_t c = static_cast<std::uint32_t>(instr.targets[0]);
        const std::uint32_t t = static_cast<std::uint32_t>(instr.targets[1]);
        m_state.CNOT(c, t, m_threads);
        return;
    }

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

    case OpType::MEASURE: {
        const int r = m_state.measure_qubit(t, m_rng);
        if (t < m_lastMeas.size()) m_lastMeas[t] = r;
        break;
    }

    default:
        throw std::runtime_error("Gate not implemented in backend yet");
    }
}
