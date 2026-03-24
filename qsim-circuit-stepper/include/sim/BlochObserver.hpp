#pragma once

#include "sim/Observer.hpp"
#include "backend/IBackendEx.hpp"

#include <memory>
#include <cstdint>

class BlochObserver final : public Observer {
public:
    BlochObserver(std::shared_ptr<IBackendEx> backend, std::uint32_t qubit_index);
    void after_step(std::size_t step, const Instruction& instr) override;

private:
    std::shared_ptr<IBackendEx> m_backend;
    std::uint32_t m_q = 0;
};
