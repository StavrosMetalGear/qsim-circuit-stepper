#pragma once

#include "backend/IStatevectorBackend.hpp"
#include "circuit/Instruction.hpp"

#include <cstdint>
#include <random>
#include <stdexcept>

// submodule headers
#include "vendor/tiny-1q-sim/include/statevector.hpp"
#include "vendor/tiny-1q-sim/include/gates.hpp"

class StatevectorBackend final : public IStatevectorBackend {
public:
    explicit StatevectorBackend(std::uint32_t num_qubits = 1, std::size_t threads = 1);

    void apply(const Instruction& instr) override;

    const qsim::StateVector& state() const override { return m_state; }
    std::uint32_t num_qubits() const override { return m_numQubits; }

private:
    std::uint32_t m_numQubits = 1;
    std::size_t   m_threads   = 1;

    qsim::StateVector m_state;
    std::mt19937_64   m_rng{std::random_device{}()};
};
