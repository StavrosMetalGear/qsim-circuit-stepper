#include "sim/MetricsObserver.hpp"

#include <complex>
#include <cmath>
#include <iostream>
#include <iomanip>

static double clamp01(double x) {
    if (x < 0.0) return 0.0;
    if (x > 1.0) return 1.0;
    return x;
}

// binary entropy H2(p) in bits
static double H2(double p) {
    p = clamp01(p);
    if (p <= 0.0 || p >= 1.0) return 0.0;
    return -(p * std::log2(p) + (1.0 - p) * std::log2(1.0 - p));
}

MetricsObserver::MetricsObserver(std::shared_ptr<IStatevectorBackend> backend,
                                 std::uint32_t metrics_qubit)
    : m_backend(std::move(backend)), m_q(metrics_qubit) {}

void MetricsObserver::after_step(std::size_t step, const Instruction&) {
    if (!m_backend) return;

    const std::uint32_t n = m_backend->num_qubits();
    if (n == 0 || m_q >= n) return;

    const auto amps = m_backend->amplitudes();
    const std::size_t dim = amps.size();
    const std::size_t mask = (std::size_t(1) << m_q);

    // Reduced 1-qubit density matrix for qubit q:
    // rho00 = Σ |psi(i0)|^2
    // rho11 = Σ |psi(i1)|^2
    // rho01 = Σ psi(i0) * conj(psi(i1))
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

    // Bloch vector from rho
    const double x = 2.0 * rho01.real();
    const double y = 2.0 * rho01.imag();
    const double z = rho00 - rho11;

    const double bloch_len = std::sqrt(x*x + y*y + z*z);

    // Purity Tr(rho^2) for 2x2 rho:
    const double purity = rho00*rho00 + rho11*rho11 + 2.0 * std::norm(rho01);

    // Coherence proxy: 2|rho01| (0=no coherence, 1=max for pure superposition)
    const double coherence = 2.0 * std::abs(rho01);

    // Entanglement entropy of qubit q with the rest (for pure global state)
    // Eigenvalues of rho are: (1 ± |r|)/2 where |r| = bloch_len
    const double lam_plus = 0.5 * (1.0 + clamp01(bloch_len));
    const double entropy_bits = H2(lam_plus);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "step " << step
              << " | q" << m_q
              << " purity=" << purity
              << " coherence=" << coherence
              << " bloch_len=" << bloch_len
              << " ent_entropy=" << entropy_bits << " bits"
              << "\n";
}
