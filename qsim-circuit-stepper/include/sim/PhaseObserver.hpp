#pragma once

#include "sim/Observer.hpp"
#include "backend/IStatevectorBackend.hpp"

#include <memory>
#include <cstddef>

class PhaseObserver final : public Observer {
public:
    // Track relative phase arg(psi[j]) - arg(psi[i])
    PhaseObserver(std::shared_ptr<IStatevectorBackend> backend,
                  std::size_t index_i = 0,
                  std::size_t index_j = 1,
                  double eps = 1e-12);

    void after_step(std::size_t step, const Instruction& instr) override;

private:
    std::shared_ptr<IStatevectorBackend> m_backend;
    std::size_t m_i = 0;
    std::size_t m_j = 1;
    double m_eps = 1e-12;
};
