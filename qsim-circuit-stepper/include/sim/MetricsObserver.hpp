#pragma once

#include "sim/Observer.hpp"
#include "backend/IStatevectorBackend.hpp"

#include <memory>
#include <cstdint>

class MetricsObserver final : public Observer {
public:
    // metrics_qubit: which qubit to analyze
    explicit MetricsObserver(std::shared_ptr<IStatevectorBackend> backend,
                             std::uint32_t metrics_qubit);

    void after_step(std::size_t step, const Instruction& instr) override;

private:
    std::shared_ptr<IStatevectorBackend> m_backend;
    std::uint32_t m_q = 0;
};
