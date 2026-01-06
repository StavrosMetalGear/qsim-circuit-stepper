// Stepper.hpp
#pragma once
#include "circuit/Circuit.hpp"
#include "sim/Observer.hpp"
#include <memory>
#include <vector>

class Stepper {
public:
    Stepper(const Circuit& c);

    void step();
    bool done() const;

    void add_observer(std::shared_ptr<Observer> obs);

private:
    const Circuit& circuit;
    size_t pc = 0;
    std::vector<std::shared_ptr<Observer>> observers;
};
#pragma once
