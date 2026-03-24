#include "sim/PhaseObserver.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>

static double wrap_pi(double x) {
    const double two_pi = 2.0 * M_PI;
    x = std::fmod(x + M_PI, two_pi);
    if (x < 0) x += two_pi;
    return x - M_PI;
}

PhaseObserver::PhaseObserver(std::shared_ptr<IStatevectorBackend> backend,
                             std::size_t index_i,
                             std::size_t index_j,
                             double eps)
    : m_backend(std::move(backend)), m_i(index_i), m_j(index_j), m_eps(eps) {}

void PhaseObserver::after_step(std::size_t step, const Instruction&) {
    if (!m_backend) return;

    std::complex<double> ai, aj;
    if (!m_backend->try_get_amplitude(m_i, ai) || !m_backend->try_get_amplitude(m_j, aj)) {
        std::cout << "step " << step << " | phase(" << m_j << "-" << m_i << ") = N/A\n";
        return;
    }

    const double mi = std::abs(ai);
    const double mj = std::abs(aj);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "step " << step << " | phase(" << m_j << "-" << m_i << ") = ";

    if (mi < m_eps || mj < m_eps) {
        std::cout << "N/A (amplitude near 0)\n";
        return;
    }

    const double phi = wrap_pi(std::arg(aj) - std::arg(ai));
    std::cout << phi << " rad\n";
}
