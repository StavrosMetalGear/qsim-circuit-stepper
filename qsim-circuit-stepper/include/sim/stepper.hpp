#pragma once
#include "circuit/Circuit.hpp"
#include "sim/Observer.hpp"
#include "backend/IBackend.hpp"
#include <memory>
#include <vector>

class Stepper {
public:
    Stepper(const Circuit& c, std::shared_ptr<IBackend> backend);

    void step();
    bool done() const;

    void add_observer(std::shared_ptr<Observer> obs);

private:
    const Circuit& circuit;
    std::shared_ptr<IBackend> backend;
    size_t pc = 0;
    std::vector<std::shared_ptr<Observer>> observers;
};


