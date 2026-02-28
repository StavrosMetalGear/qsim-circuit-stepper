#include "sim/TraceObserver.hpp"

#include <complex>
#include <iomanip>
#include <cmath>
#include <sstream>

TraceObserver::TraceObserver(std::shared_ptr<IStatevectorBackend> backend,
                             std::string csv_path,
                             std::uint32_t bloch_qubit,
                             std::size_t phase_i,
                             std::size_t phase_j,
                             double eps)
    : m_backend(std::move(backend)),
      m_out(std::move(csv_path)),
      m_q(bloch_qubit),
      m_i(phase_i),
      m_j(phase_j),
      m_eps(eps)
{
    // CSV header
    m_out << "step,op,targets,params,bloch_q,x,y,z,p0,p1,phase_i,phase_j,phase_rad\n";
    m_out.flush();
}

TraceObserver::~TraceObserver() {
    if (m_out.is_open()) m_out.flush();
}

const char* TraceObserver::op_name(OpType t) {
    switch (t) {
        case OpType::H: return "H";
        case OpType::X: return "X";
        case OpType::Y: return "Y";
        case OpType::Z: return "Z";
        case OpType::RX: return "RX";
        case OpType::RY: return "RY";
        case OpType::RZ: return "RZ";
        case OpType::CNOT: return "CNOT";
        case OpType::MEASURE: return "MEASURE";
        default: return "?";
    }
}

double TraceObserver::wrap_pi(double x) {
    const double two_pi = 2.0 * M_PI;
    x = std::fmod(x + M_PI, two_pi);
    if (x < 0) x += two_pi;
    return x - M_PI;
}

void TraceObserver::reduced_bloch_and_probs(const std::vector<std::complex<double>>& amps,
                                            std::uint32_t n,
                                            std::uint32_t q,
                                            double& x, double& y, double& z,
                                            double& p0, double& p1) const
{
    const std::size_t dim = amps.size();
    const std::size_t mask = (std::size_t(1) << q);

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

    x = 2.0 * rho01.real();
    y = 2.0 * rho01.imag();
    z = rho00 - rho11;

    p0 = rho00;
    p1 = rho11;
}

bool TraceObserver::relative_phase(const std::vector<std::complex<double>>& amps,
                                   double& out_phi) const
{
    if (m_i >= amps.size() || m_j >= amps.size()) return false;

    const auto ai = amps[m_i];
    const auto aj = amps[m_j];

    if (std::abs(ai) < m_eps || std::abs(aj) < m_eps) return false;

    out_phi = wrap_pi(std::arg(aj) - std::arg(ai));
    return true;
}

void TraceObserver::after_step(std::size_t step, const Instruction& instr) {
    if (!m_backend || !m_out.is_open()) return;

    const std::uint32_t n = m_backend->num_qubits();
    auto amps = m_backend->amplitudes();

    double x=0, y=0, z=0, p0=0, p1=0;
    if (n > 0 && m_q < n) {
        reduced_bloch_and_probs(amps, n, m_q, x, y, z, p0, p1);
    }

    double phi = 0.0;
    const bool has_phi = relative_phase(amps, phi);

    // targets as "0;1;2"
    std::ostringstream tss;
    for (std::size_t k = 0; k < instr.targets.size(); ++k) {
        if (k) tss << ';';
        tss << instr.targets[k];
    }

    // params as "0.1;0.2"
    std::ostringstream pss;
    for (std::size_t k = 0; k < instr.params.size(); ++k) {
        if (k) pss << ';';
        pss << instr.params[k];
    }

    m_out << step << ","
          << op_name(instr.type) << ","
          << "\"" << tss.str() << "\"" << ","
          << "\"" << pss.str() << "\"" << ","
          << m_q << ","
          << std::fixed << std::setprecision(10)
          << x << "," << y << "," << z << ","
          << p0 << "," << p1 << ","
          << m_i << "," << m_j << ",";

    if (has_phi) m_out << phi;
    else m_out << ""; // empty = N/A

    m_out << "\n";
    m_out.flush();
}
