#include "backend/StatevectorBackend.hpp"

#include <cmath>
#include <stdexcept>

using qsim::Complex;
using qsim::Gate2x2;

static Gate2x2 Y_gate() {
    // [[0, -i],
    //  [i,  0]]
    return {{
        { Complex(0,0), Complex(0,-1) },
        { Complex(0,1), Complex(0,0)  }
    }};
}

static Gate2x2 RX_gate(double theta) {
    // RX(theta) = cos(theta/2) I - i sin(theta/2) X
    const double c = std::cos(theta * 0.5);
    const double s = std::sin(theta * 0.5);
    return {{
        { Complex(c,0), Complex(0,-s) },
        { Complex(0,-s), Complex(c,0) }
    }};
}

static Gate2x2 RY_gate(double theta) {
    // RY(theta) =
    // [[ cos(t/2), -sin(t/2)],
    //  [ sin(t/2),  cos(t/2)]]
    const double c = std::cos(theta * 0.5);
    const double s = std::sin(theta * 0.5);
    return {{
        { Complex(c,0), Complex(-s,0) },
        { Complex(s,0), Complex(c,0)  }
    }};
}

static Gate2x2 RZ_gate(double theta) {
    // RZ(theta) = diag(e^{-i t/2}, e^{+i t/2})
    const double h = theta * 0.5;
    return {{
        { Complex(std::cos(-h), std::sin(-h)), Complex(0,0) },
        { Complex(0,0), Complex(std::cos(h), std::sin(h)) }
    }};
}

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
    // Two-qubit
    if (instr.type == OpType::CNOT) {
        if (instr.targets.size() != 2) {
            throw std::runtime_error("CNOT requires 2 targets (control, target)");
        }
        const std::uint32_t c = static_cast<std::uint32_t>(instr.targets[0]);
        const std::uint32_t t = static_cast<std::uint32_t>(instr.targets[1]);
        m_state.CNOT(c, t, m_threads);
        return;
    }

    // One-qubit ops
    if (instr.targets.empty())
        throw std::runtime_error("Instruction has no targets");

    const std::uint32_t t = static_cast<std::uint32_t>(instr.targets[0]);

    auto need_angle = [&]() -> double {
        if (instr.params.empty()) {
            throw std::runtime_error("Rotation gate requires angle param (params[0])");
        }
        return instr.params[0];
    };

    switch (instr.type) {
    case OpType::H:
        m_state.H(t, m_threads);
        break;
    case OpType::X:
        m_state.X(t, m_threads);
        break;
    case OpType::Y:
        m_state.apply_gate_1q(t, Y_gate(), m_threads);
        break;
    case OpType::Z:
        m_state.Z(t, m_threads);
        break;

    case OpType::RX: {
        const double theta = need_angle();
        m_state.apply_gate_1q(t, RX_gate(theta), m_threads);
        break;
    }
    case OpType::RY: {
        const double theta = need_angle();
        m_state.apply_gate_1q(t, RY_gate(theta), m_threads);
        break;
    }
    case OpType::RZ: {
        const double theta = need_angle();
        m_state.apply_gate_1q(t, RZ_gate(theta), m_threads);
        break;
    }

    case OpType::MEASURE: {
        const int r = m_state.measure_qubit(t, m_rng);
        if (t < m_lastMeas.size()) m_lastMeas[t] = r;
        break;
    }

    default:
        throw std::runtime_error("Unsupported operation in backend");
    }
}
