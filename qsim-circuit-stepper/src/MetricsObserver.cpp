#include "sim/MetricsObserver.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>

static double clamp01(double x){ return x<0?0:(x>1?1:x); }
static double H2(double p){
    p = clamp01(p);
    if (p<=0.0 || p>=1.0) return 0.0;
    return -(p*std::log2(p) + (1.0-p)*std::log2(1.0-p));
}

MetricsObserver::MetricsObserver(std::shared_ptr<IBackendEx> backend, std::uint32_t qubit)
    : m_backend(std::move(backend)), m_q(qubit) {}

void MetricsObserver::after_step(std::size_t step, const Instruction&) {
    if (!m_backend) return;
    if (m_q >= m_backend->num_qubits()) return;

    const auto r = m_backend->reduced_density_1q(m_q);

    const double rho00 = r[0][0].real();
    const double rho11 = r[1][1].real();
    const auto rho01 = r[0][1];

    const double x = 2.0 * rho01.real();
    const double y = 2.0 * rho01.imag();
    const double z = rho00 - rho11;

    const double bloch_len = std::sqrt(x*x + y*y + z*z);
    const double purity = rho00*rho00 + rho11*rho11 + 2.0*std::norm(rho01);
    const double coherence = 2.0 * std::abs(rho01);

    const double lam_plus = 0.5 * (1.0 + clamp01(bloch_len));
    const double ent_entropy = H2(lam_plus);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "step " << step << " | q" << m_q
              << " purity=" << purity
              << " coherence=" << coherence
              << " bloch_len=" << bloch_len
              << " ent_entropy=" << ent_entropy << " bits\n";
}
