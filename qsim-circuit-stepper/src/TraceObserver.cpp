#include "sim/TraceObserver.hpp"

#include <cmath>
#include <iomanip>
#include <limits>

static double clamp01(double x){ return x<0?0:(x>1?1:x); }
static double H2(double p){
    p = clamp01(p);
    if (p<=0.0 || p>=1.0) return 0.0;
    return -(p*std::log2(p) + (1.0-p)*std::log2(1.0-p));
}
static double wrap_pi(double x) {
    const double two_pi = 2.0 * M_PI;
    x = std::fmod(x + M_PI, two_pi);
    if (x < 0) x += two_pi;
    return x - M_PI;
}

TraceObserver::TraceObserver(std::shared_ptr<IBackendEx> backend,
                             const std::string& csv_path,
                             std::uint32_t qubit_for_metrics,
                             std::size_t phase_i,
                             std::size_t phase_j)
    : m_backend(std::move(backend)),
      m_out(csv_path),
      m_q(qubit_for_metrics),
      m_i(phase_i),
      m_j(phase_j)
{
    if (!m_out) {
        throw std::runtime_error("TraceObserver: could not open " + csv_path);
    }
    m_out << std::fixed << std::setprecision(10);
}

void TraceObserver::after_step(std::size_t step, const Instruction&) {
    if (!m_backend) return;
    if (m_q >= m_backend->num_qubits()) return;

    if (!m_wrote_header) {
        m_out << "step,x,y,z,purity,coherence,bloch_len,entropy_bits,phase_rad\n";
        m_wrote_header = true;
    }

    const auto r = m_backend->reduced_density_1q(m_q);

    const double rho00 = r[0][0].real();
    const double rho11 = r[1][1].real();
    const auto   rho01 = r[0][1];

    const double x = 2.0 * rho01.real();
    const double y = 2.0 * rho01.imag();
    const double z = rho00 - rho11;

    const double bloch_len = std::sqrt(x*x + y*y + z*z);
    const double purity = rho00*rho00 + rho11*rho11 + 2.0*std::norm(rho01);
    const double coherence = 2.0 * std::abs(rho01);

    const double lam_plus = 0.5 * (1.0 + clamp01(bloch_len));
    const double entropy = H2(lam_plus);

    double phase = std::numeric_limits<double>::quiet_NaN();
    Complex ai, aj;
    if (m_backend->try_get_amplitude(m_i, ai) && m_backend->try_get_amplitude(m_j, aj)) {
        if (std::abs(ai) > 1e-12 && std::abs(aj) > 1e-12) {
            phase = wrap_pi(std::arg(aj) - std::arg(ai));
        }
    }

    m_out << step << ","
          << x << "," << y << "," << z << ","
          << purity << "," << coherence << "," << bloch_len << ","
          << entropy << ",";

    if (!std::isnan(phase)) m_out << phase;
    m_out << "\n";
    m_out.flush();
}
