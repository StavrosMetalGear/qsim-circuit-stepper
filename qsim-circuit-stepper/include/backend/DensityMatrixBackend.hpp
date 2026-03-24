#pragma once

#include "backend/IBackendEx.hpp"
#include "circuit/Instruction.hpp"

#include <vector>
#include <random>
#include <array>

class DensityMatrixBackend final : public IBackendEx {
public:
    explicit DensityMatrixBackend(std::uint32_t num_qubits,
                                  double depolarize_p = 0.0,
                                  double dephase_gamma = 0.0,
                                  double amp_damp_gamma = 0.0);

    void apply(const Instruction& instr) override;

    std::uint32_t num_qubits() const override { return m_n; }

    void reset() override;
    void set_seed(std::uint64_t seed) override;
    const std::vector<int>& last_measurements() const override { return m_lastMeas; }

    Rho2 reduced_density_1q(std::uint32_t q) const override;

private:
    std::uint32_t m_n = 0;
    std::size_t   m_dim = 0;

    std::vector<Complex> m_rho; // row-major dim*dim

    double m_depol = 0.0;
    double m_dephase = 0.0;
    double m_ampdamp = 0.0;

    std::mt19937_64 m_rng;
    std::vector<int> m_lastMeas;

private:
    inline Complex& rho(std::size_t r, std::size_t c) { return m_rho[r*m_dim + c]; }
    inline const Complex& rho(std::size_t r, std::size_t c) const { return m_rho[r*m_dim + c]; }

    using U2 = std::array<std::array<Complex,2>,2>;

    void apply_linear_1q(std::uint32_t q, const U2& A, const U2& B); // rho <- (A) rho (B)
    void apply_unitary_1q(std::uint32_t q, const U2& U);             // rho <- U rho U†
    void apply_cnot(std::uint32_t control, std::uint32_t target);

    void apply_depolarizing(std::uint32_t q, double p);
    void apply_dephasing(std::uint32_t q, double gamma);
    void apply_amplitude_damping(std::uint32_t q, double gamma);

    int measure_qubit(std::uint32_t q);

    static U2 X();
    static U2 Y();
    static U2 Z();
    static U2 H();
    static U2 RX(double t);
    static U2 RY(double t);
    static U2 RZ(double t);
};
