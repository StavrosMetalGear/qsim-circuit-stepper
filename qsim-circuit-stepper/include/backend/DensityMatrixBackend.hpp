#pragma once

#include "backend/IBackendEx.hpp"
#include "circuit/Instruction.hpp"

#include <vector>
#include <random>

class DensityMatrixBackend final : public IBackendEx {
public:
    explicit DensityMatrixBackend(std::uint32_t num_qubits, double depolarize_p = 0.0);

    void apply(const Instruction& instr) override;

    std::uint32_t num_qubits() const override { return m_n; }

    void reset() override;
    void set_seed(std::uint64_t seed) override;
    const std::vector<int>& last_measurements() const override { return m_lastMeas; }

    Rho2 reduced_density_1q(std::uint32_t q) const override;

private:
    std::uint32_t m_n = 0;
    std::size_t   m_dim = 0;

    // density matrix stored row-major dim*dim
    std::vector<Complex> m_rho;

    // noise: depolarizing per gate (applied after each non-measure gate)
    double m_depol = 0.0;

    std::mt19937_64 m_rng;
    std::vector<int> m_lastMeas;

private:
    inline Complex& rho(std::size_t r, std::size_t c) { return m_rho[r*m_dim + c]; }
    inline const Complex& rho(std::size_t r, std::size_t c) const { return m_rho[r*m_dim + c]; }

    void apply_unitary_1q(std::uint32_t q, const std::array<std::array<Complex,2>,2>& U);
    void apply_cnot(std::uint32_t control, std::uint32_t target);

    void apply_depolarizing(std::uint32_t q, double p);

    int measure_qubit(std::uint32_t q);

    static std::array<std::array<Complex,2>,2> X();
    static std::array<std::array<Complex,2>,2> Y();
    static std::array<std::array<Complex,2>,2> Z();
    static std::array<std::array<Complex,2>,2> H();
    static std::array<std::array<Complex,2>,2> RX(double t);
    static std::array<std::array<Complex,2>,2> RY(double t);
    static std::array<std::array<Complex,2>,2> RZ(double t);
};
