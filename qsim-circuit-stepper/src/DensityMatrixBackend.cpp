#include "backend/DensityMatrixBackend.hpp"

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <array>

static inline std::size_t bitmask(std::uint32_t q) { return (std::size_t(1) << q); }
using U2 = std::array<std::array<Complex,2>,2>;

static inline U2 make2(const Complex& a00, const Complex& a01,
                       const Complex& a10, const Complex& a11) {
    return U2{{
        std::array<Complex,2>{a00, a01},
        std::array<Complex,2>{a10, a11}
    }};
}

DensityMatrixBackend::DensityMatrixBackend(std::uint32_t num_qubits, double depolarize_p)
    : m_n(num_qubits),
      m_dim(std::size_t(1) << num_qubits),
      m_rho(m_dim * m_dim, Complex(0,0)),
      m_depol(depolarize_p),
      m_rng(0),
      m_lastMeas(num_qubits, -1)
{
    reset();
}

void DensityMatrixBackend::reset() {
    std::fill(m_rho.begin(), m_rho.end(), Complex(0,0));
    rho(0,0) = Complex(1,0); // |00..0><00..0|
    std::fill(m_lastMeas.begin(), m_lastMeas.end(), -1);
}

void DensityMatrixBackend::set_seed(std::uint64_t seed) {
    m_rng.seed(seed);
}

U2 DensityMatrixBackend::X() { return make2({0,0},{1,0},{1,0},{0,0}); }
U2 DensityMatrixBackend::Y() { return make2({0,0},{0,-1},{0,1},{0,0}); }
U2 DensityMatrixBackend::Z() { return make2({1,0},{0,0},{0,0},{-1,0}); }

U2 DensityMatrixBackend::H() {
    double inv = 1.0/std::sqrt(2.0);
    return make2({inv,0},{inv,0},{inv,0},{-inv,0});
}
U2 DensityMatrixBackend::RX(double t) {
    double c = std::cos(t*0.5), s = std::sin(t*0.5);
    return make2({c,0},{0,-s},{0,-s},{c,0});
}
U2 DensityMatrixBackend::RY(double t) {
    double c = std::cos(t*0.5), s = std::sin(t*0.5);
    return make2({c,0},{-s,0},{s,0},{c,0});
}
U2 DensityMatrixBackend::RZ(double t) {
    double h = t*0.5;
    return make2({std::cos(-h),std::sin(-h)},{0,0},{0,0},{std::cos(h),std::sin(h)});
}

// Apply left and right multiplication: rho <- (U ⊗ I) rho (U† ⊗ I)
void DensityMatrixBackend::apply_unitary_1q(std::uint32_t q, const U2& U) {
    const std::size_t mask = bitmask(q);

    // Left multiply: for each column, update pairs of rows (i0,i1)
    for (std::size_t col = 0; col < m_dim; ++col) {
        for (std::size_t i0 = 0; i0 < m_dim; ++i0) {
            if ((i0 & mask) != 0) continue;
            std::size_t i1 = i0 | mask;

            const Complex a0 = rho(i0,col);
            const Complex a1 = rho(i1,col);

            rho(i0,col) = U[0][0]*a0 + U[0][1]*a1;
            rho(i1,col) = U[1][0]*a0 + U[1][1]*a1;
        }
    }

    // Right multiply by U†
    for (std::size_t row = 0; row < m_dim; ++row) {
        for (std::size_t j0 = 0; j0 < m_dim; ++j0) {
            if ((j0 & mask) != 0) continue;
            std::size_t j1 = j0 | mask;

            const Complex b0 = rho(row,j0);
            const Complex b1 = rho(row,j1);

            rho(row,j0) = b0*std::conj(U[0][0]) + b1*std::conj(U[0][1]);
            rho(row,j1) = b0*std::conj(U[1][0]) + b1*std::conj(U[1][1]);
        }
    }
}

void DensityMatrixBackend::apply_cnot(std::uint32_t control, std::uint32_t target) {
    if (control == target) throw std::runtime_error("CNOT control==target");
    const std::size_t cm = bitmask(control);
    const std::size_t tm = bitmask(target);

    auto map_index = [&](std::size_t idx)->std::size_t{
        if (idx & cm) return idx ^ tm;
        return idx;
    };

    // Left multiply by CNOT: permute rows
    std::vector<Complex> tmp(m_dim*m_dim, Complex(0,0));
    for (std::size_t i = 0; i < m_dim; ++i) {
        std::size_t ip = map_index(i);
        for (std::size_t j = 0; j < m_dim; ++j) {
            tmp[ip*m_dim + j] = rho(i,j);
        }
    }
    m_rho.swap(tmp);

    // Right multiply by CNOT (same)
    tmp.assign(m_dim*m_dim, Complex(0,0));
    for (std::size_t j = 0; j < m_dim; ++j) {
        std::size_t jp = map_index(j);
        for (std::size_t i = 0; i < m_dim; ++i) {
            tmp[i*m_dim + jp] = rho(i,j);
        }
    }
    m_rho.swap(tmp);
}

void DensityMatrixBackend::apply_depolarizing(std::uint32_t q, double p) {
    if (p <= 0.0) return;
    if (p > 1.0) p = 1.0;

    auto rho0 = m_rho;

    auto conj_apply = [&](const U2& P)->std::vector<Complex>{
        m_rho = rho0;
        apply_unitary_1q(q, P);
        return m_rho;
    };

    auto x = conj_apply(X());
    auto y = conj_apply(Y());
    auto z = conj_apply(Z());

    m_rho = rho0;
    for (std::size_t k = 0; k < m_rho.size(); ++k) {
        m_rho[k] = (1.0 - p)*rho0[k] + (p/3.0)*(x[k] + y[k] + z[k]);
    }
}

int DensityMatrixBackend::measure_qubit(std::uint32_t q) {
    const std::size_t mask = bitmask(q);

    double p0 = 0.0, p1 = 0.0;
    for (std::size_t i = 0; i < m_dim; ++i) {
        double diag = rho(i,i).real();
        if ((i & mask) == 0) p0 += diag;
        else p1 += diag;
    }

    if (p0 < 0) p0 = 0;
    if (p1 < 0) p1 = 0;
    double s = p0 + p1;
    if (s <= 0) { p0 = 1.0; p1 = 0.0; s = 1.0; }
    p0 /= s; p1 /= s;

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    const double r = dist(m_rng);
    const int outcome = (r < p0) ? 0 : 1;
    const double p = (outcome == 0) ? p0 : p1;

    for (std::size_t i = 0; i < m_dim; ++i) {
        for (std::size_t j = 0; j < m_dim; ++j) {
            const int bi = ((i & mask) ? 1 : 0);
            const int bj = ((j & mask) ? 1 : 0);
            if (bi != outcome || bj != outcome) rho(i,j) = Complex(0,0);
            else rho(i,j) /= p;
        }
    }

    return outcome;
}

Rho2 DensityMatrixBackend::reduced_density_1q(std::uint32_t q) const {
    const std::size_t mask = bitmask(q);

    Complex r00(0,0), r11(0,0), r01(0,0);
    for (std::size_t i0 = 0; i0 < m_dim; ++i0) {
        if ((i0 & mask) != 0) continue;
        std::size_t i1 = i0 | mask;
        r00 += rho(i0,i0);
        r11 += rho(i1,i1);
        r01 += rho(i0,i1);
    }

    Rho2 out{};
    out[0][0] = r00;
    out[1][1] = r11;
    out[0][1] = r01;
    out[1][0] = std::conj(r01);
    return out;
}

void DensityMatrixBackend::apply(const Instruction& instr) {
    if (instr.targets.empty()) throw std::runtime_error("Instruction has no targets");

    if (instr.type == OpType::CNOT) {
        if (instr.targets.size() != 2) throw std::runtime_error("CNOT requires 2 targets");
        apply_cnot((std::uint32_t)instr.targets[0], (std::uint32_t)instr.targets[1]);
        if (m_depol > 0.0) {
            apply_depolarizing((std::uint32_t)instr.targets[0], m_depol);
            apply_depolarizing((std::uint32_t)instr.targets[1], m_depol);
        }
        return;
    }

    const std::uint32_t q = (std::uint32_t)instr.targets[0];

    auto need_angle = [&]()->double{
        if (instr.params.empty()) throw std::runtime_error("Rotation gate needs params[0]");
        return instr.params[0];
    };

    switch (instr.type) {
    case OpType::H: apply_unitary_1q(q, H()); break;
    case OpType::X: apply_unitary_1q(q, X()); break;
    case OpType::Y: apply_unitary_1q(q, Y()); break;
    case OpType::Z: apply_unitary_1q(q, Z()); break;
    case OpType::RX: apply_unitary_1q(q, RX(need_angle())); break;
    case OpType::RY: apply_unitary_1q(q, RY(need_angle())); break;
    case OpType::RZ: apply_unitary_1q(q, RZ(need_angle())); break;

    case OpType::MEASURE: {
        int r = measure_qubit(q);
        if (q < m_lastMeas.size()) m_lastMeas[q] = r;
        return;
    }

    default:
        throw std::runtime_error("Unsupported op in DensityMatrixBackend");
    }

    if (m_depol > 0.0) apply_depolarizing(q, m_depol);
}
