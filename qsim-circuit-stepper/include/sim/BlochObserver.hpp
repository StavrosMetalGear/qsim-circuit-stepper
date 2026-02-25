#pragma once

#include "sim/Observer.hpp"
#include "backend/IStatevectorBackend.hpp"

#include <memory>
#include <cstdint>

class BlochObserver final : public Observer {
public:
    // Track Bloch vector for a chosen qubit index (0 = least significant bit)
    BlochObserver(std::shared_ptr<IStatevectorBackend> backend, std::uint32_t qubit_index);

    void after_step(std::size_t step, const Instruction& instr) override;

private:
    std::shared_ptr<IStatevectorBackend> m_backend;
    std::uint32_t m_q = 0;
};
