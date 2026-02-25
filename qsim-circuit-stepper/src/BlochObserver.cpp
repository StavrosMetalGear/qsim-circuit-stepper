#include "sim/BlochObserver.hpp"

#include <complex>
#include <iostream>
#include <iomanip>
#include <stdexcept>

BlochObserver::BlochObserver(std::shared_ptr<IStatevectorBackend> backend, std::uint32_t qubit_index)
    : m_backend(std::move(backend)), m_q(qubit_index) {}

void BlochObserver::after_step(std::size_t step, const Instruction& /*instr*/) {
    if (!m_backend) return;

    const std::uint32_t n = m_backend->num_qubits();
    if (n == 0 || m_q >= n) return;

    const auto amps = m_backend->amplitudes();
    const std::size_t dim = amps.size();
    if (dim == 0) return;

    const std::size_t mask = (std::size_t(1) << m_q);

    // Reduced 1-qubit density matrix elements for qubit m_q:
    // rho00 = Σ |psi(i0)|^2
    // rho11 = Σ |psi(i1)|^2
    // rho01 = Σ psi(i0) * conj(psi(i1))
    double rho00 = 0.0;
    double rho11 = 0.0;
    std::complex<double> rho01{0.0, 0.0};

    // Loop all basis indices where bit q = 0 and pair with bit q = 1
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

    // Bloch vector from reduced density matrix:
    // x = 2 Re(rho01), y = 2 Im(rho01), z = rho00 - rho11
    const double x = 2.0 * rho01.real();
    const double y = 2.0 * rho01.imag();
    const double z = rho00 - rho11;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "step " << step
              << " | Bloch q" << m_q
              << " = (" << x << ", " << y << ", " << z << ")\n";
}
