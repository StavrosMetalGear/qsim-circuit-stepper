#pragma once
#include "sim/Observer.hpp"
#include "backend/IStatevectorBackend.hpp"

#include <memory>

class BlochObserver : public Observer {
public:
    explicit BlochObserver(std::shared_ptr<IStatevectorBackend> b);

    void before_step(std::size_t /*pc*/, const Instruction& /*instr*/) override {}
    void after_step(std::size_t pc, const Instruction& instr) override;

private:
    std::shared_ptr<IStatevectorBackend> backend;
};
