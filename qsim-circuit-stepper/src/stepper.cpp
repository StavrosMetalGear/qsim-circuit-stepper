#include "sim/stepper.hpp"
#include "backend/IStatevectorBackend.hpp"

#include <iostream>
#include <iomanip>
#include <complex>
#include <memory>

Stepper::Stepper(const Circuit& c, std::shared_ptr<IBackend> b)
    : circuit(c), backend(std::move(b)) {}

void Stepper::add_observer(std::shared_ptr<Observer> obs) {
    observers.push_back(std::move(obs));
}

bool Stepper::done() const {
    return pc >= circuit.size();
}

void Stepper::step() {
    if (done()) return;

    const auto& instr = circuit[pc];

    for (auto& o : observers) o->before_step(pc, instr);

    backend->apply(instr);

    // ---- Phase 2.1: print 1-qubit snapshot after each step ----
    auto sv = std::dynamic_pointer_cast<IStatevectorBackend>(backend);
    if (sv && sv->num_qubits() == 1) {
        auto amps = sv->amplitudes();
        if (amps.size() == 2) {
            auto a0 = amps[0];
            auto a1 = amps[1];
            double p0 = std::norm(a0);
            double p1 = std::norm(a1);

            std::cout << std::fixed << std::setprecision(6);
            std::cout << "step " << pc << " | "
                      << "a0=(" << a0.real() << (a0.imag()>=0?"+":"") << a0.imag() << "i) "
                      << "a1=(" << a1.real() << (a1.imag()>=0?"+":"") << a1.imag() << "i) "
                      << "p0=" << p0 << " p1=" << p1
                      << "\n";
        }
    }

    for (auto& o : observers) o->after_step(pc, instr);

    ++pc;
}
