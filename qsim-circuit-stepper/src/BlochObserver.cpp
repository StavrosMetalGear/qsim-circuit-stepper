#include "sim/BlochObserver.hpp"
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>

BlochObserver::BlochObserver(std::shared_ptr<IStatevectorBackend> backend, std::uint32_t qubit)
    : m_backend(std::move(backend)), m_q(qubit) {}

void BlochObserver::after_step(std::size_t step, const Instruction&) {
    if (!m_backend) return;

    const std::uint32_t n = m_backend->num_qubits();
    if (n == 0 || m_q >= n) return;

    const auto& amps = m_backend->amplitudes_ref();
    const std::size_t dim = amps.size();
    const std::size_t mask = (std::size_t(1) << m_q);

    // Reduced 1-qubit density matrix elements
    double rho00 = 0.0;
    double rho11 = 0.0;
    std::complex<double> rho01{0.0, 0.0};

    for (std::size_t i0 = 0; i0 < dim; ++i0) {
        if ((i0 & mask) != 0) continue;
        const std::size_t i1 = i0 | mask;
        if (i1 >= dim) continue;

        const auto a0 = amps[i0];
        const auto a1 = amps[i1];
        rho00 += std::norm(a0);
        rho11 += std::norm(a1);
        rho01 += a0 * std::conj(a1);
    }

    // Bloch vector components
    const double x = 2.0 * rho01.real();
    const double y = 2.0 * rho01.imag();
    const double z = rho00 - rho11;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "step " << step
              << " | Bloch q" << m_q
              << " = (" << x << ", " << y << ", " << z << ")"
              << "\n";
}
