#pragma once
#include "backend/IStatevectorBackend.hpp"
#include "circuit/Instruction.hpp"
#include <memory>

// Include your Repo #1 header that defines Statevector.
// Example (change this to your real path once you copy/vendor Repo #1):
// #include "vendor/tiny-qsim-core/include/Statevector.hpp"
#include "Statevector.hpp" // <-- change this include to your real header

class StatevectorBackend final : public IStatevectorBackend {
public:
    explicit StatevectorBackend(int num_qubits = 1);

    void apply(const Instruction& instr) override;

    const Statevector& state() const override { return m_state; }
    int num_qubits() const override { return m_numQubits; }

private:
    int m_numQubits = 1;
    Statevector m_state;

private:
    void apply_1q_gate(OpType type, int target, double param = 0.0);
};
#pragma once
