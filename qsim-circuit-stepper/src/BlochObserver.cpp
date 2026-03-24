#include "sim/BlochObserver.hpp"
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>

BlochObserver::BlochObserver(std::shared_ptr<IBackendEx> backend, std::uint32_t qubit)
    : m_backend(std::move(backend)), m_q(qubit) {}

void BlochObserver::after_step(std::size_t step, const Instruction&) {
    if (!m_backend) return;
    if (m_q >= m_backend->num_qubits()) return;

    const auto r = m_backend->reduced_density_1q(m_q);

    // Bloch: x = 2 Re(r01), y = 2 Im(r01), z = r00 - r11
    const double x = 2.0 * r[0][1].real();
    const double y = 2.0 * r[0][1].imag();
    const double z = (r[0][0] - r[1][1]).real();

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "step " << step << " | Bloch q" << m_q
              << " = (" << x << ", " << y << ", " << z << ")\n";
}
