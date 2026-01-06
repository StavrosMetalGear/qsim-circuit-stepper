// Stepper.cpp
#include "sim/Stepper.hpp"

Stepper::Stepper(const Circuit& c) : circuit(c) {}

void Stepper::add_observer(std::shared_ptr<Observer> obs) {
    observers.push_back(obs);
}

bool Stepper::done() const {
    return pc >= circuit.size();
}

void Stepper::step() {
    if (done()) return;

    const auto& instr = circuit[pc];

    for (auto& o : observers)
        o->before_step(pc, instr);

    // backend.apply(instr)  <-- later

    for (auto& o : observers)
        o->after_step(pc, instr);

    pc++;
}
