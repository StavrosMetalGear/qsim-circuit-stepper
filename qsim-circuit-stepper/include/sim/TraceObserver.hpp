#pragma once

#include "sim/Observer.hpp"
#include "backend/IStatevectorBackend.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>

class TraceObserver final : public Observer {
public:
    TraceObserver(std::shared_ptr<IStatevectorBackend> backend,
                  std::string csv_path,
                  std::uint32_t bloch_qubit = 0,
                  std::size_t phase_i = 0,
                  std::size_t phase_j = 1,
                  double eps = 1e-12);

    ~TraceObserver() override;

    void after_step(std::size_t step, const Instruction& instr) override;

private:
    std::shared_ptr<IStatevectorBackend> m_backend;
    std::ofstream m_out;

    std::uint32_t m_q = 0;
    std::size_t m_i = 0;
    std::size_t m_j = 1;
    double m_eps = 1e-12;

private:
    static const char* op_name(OpType t);

    void reduced_bloch_and_probs(const std::vector<std::complex<double>>& amps,
                                 std::uint32_t n,
                                 std::uint32_t q,
                                 double& x, double& y, double& z,
                                 double& p0, double& p1) const;

    bool relative_phase(const std::vector<std::complex<double>>& amps,
                        double& out_phi) const;

    static double wrap_pi(double x);
};
