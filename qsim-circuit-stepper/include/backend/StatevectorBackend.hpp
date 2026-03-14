#pragma once

#include "backend/IStatevectorBackend.hpp"
#include "circuit/Instruction.hpp"

#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>
#include <complex>

// tiny sim headers (via CMake include dirs)
#include "statevector.hpp"
#include "gates.hpp"

class StatevectorBackend final : public IStatevectorBackend {
public:
    explicit StatevectorBackend(std::uint32_t num_qubits = 1, std::size_t threads = 1);

    void apply(const Instruction& instr) override;

    const qsim::StateVector& state() const override { return m_state; }

    // Old copy API
    std::vector<std::complex<double>> amplitudes() const override;

    // New no-copy API
    const std::vector<std::complex<double>>& amplitudes_ref() const override;

    std::uint32_t num_qubits() const override { return m_numQubits; }

    void reset() override;
    void set_seed(std::uint64_t seed) override;
    const std::vector<int>& last_measurements() const override { return m_lastMeas; }

private:
    std::uint32_t m_numQubits = 1;
    std::size_t   m_threads   = 1;

    qsim::StateVector m_state;
    std::mt19937_64   m_rng;

    std::vector<int> m_lastMeas;
};
