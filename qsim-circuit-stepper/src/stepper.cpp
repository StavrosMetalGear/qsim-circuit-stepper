#include "sim/stepper.hpp"

Stepper::Stepper(const Circuit& c, std::shared_ptr<IBackendEx> b)
    : circuit(c), backend(std::move(b)), pc(0) {}

bool Stepper::done() const {
    return pc >= circuit.size();
}

void Stepper::add_observer(std::shared_ptr<Observer> obs) {
    observers.push_back(std::move(obs));
}

void Stepper::step() {
    if (done()) return;

    const Instruction instr = circuit[pc];

    for (auto& o : observers) o->before_step(pc, instr);
    backend->apply(instr);
    for (auto& o : observers) o->after_step(pc, instr);

    pc++;
}
