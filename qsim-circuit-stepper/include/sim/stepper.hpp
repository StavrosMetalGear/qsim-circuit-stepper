#pragma once
#include "circuit/Circuit.hpp"
#include "sim/Observer.hpp"
#include "backend/IBackend.hpp"

#include <memory>
#include <vector>
#include <cstddef>

class Stepper {
public:
    Stepper(const Circuit& c, std::shared_ptr<IBackend> backend);

    void step();
    bool done() const;

    void add_observer(std::shared_ptr<Observer> obs);

    // Debugger helpers
    std::size_t current_pc() const { return pc; }
    Instruction peek() const { return circuit[pc]; } // only call when !done()

private:
    const Circuit& circuit;
    std::shared_ptr<IBackend> backend;

    std::size_t pc = 0;
    std::vector<std::shared_ptr<Observer>> observers;
};
