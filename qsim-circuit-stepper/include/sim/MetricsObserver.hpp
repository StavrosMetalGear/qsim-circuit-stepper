#pragma once

#include "sim/Observer.hpp"
#include "backend/IBackendEx.hpp"

#include <memory>
#include <cstdint>

class MetricsObserver final : public Observer {
public:
    explicit MetricsObserver(std::shared_ptr<IBackendEx> backend, std::uint32_t qubit);
    void after_step(std::size_t step, const Instruction& instr) override;

private:
    std::shared_ptr<IBackendEx> m_backend;
    std::uint32_t m_q = 0;
};
