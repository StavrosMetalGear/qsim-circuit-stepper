#pragma once
#include "backend/IStatevectorBackend.hpp"
#include "circuit/Instruction.hpp"
#include <random>
#include <cstdint>

// Prefer the VS include-dir setting so these work:
#include "statevector.hpp"
#include "gates.hpp"

class StatevectorBackend final : public IStatevectorBackend {
public:
    explicit StatevectorBackend(std::uint32_t num_qubits, std::size_t threads = 1);

    void apply(const Instruction& instr) override;

    const qsim::StateVector& state() const override { return m_state; }
    std::uint32_t num_qubits() const override { return m_numQubits; }

private:
    std::uint32_t m_numQubits;
    std::size_t m_threads;
    qsim::StateVector m_state;

    std::mt19937_64 m_rng{ std::random_device{}() };
};
