#include "circuit/Circuit.hpp"
#include "sim/stepper.hpp"
#include "backend/StatevectorBackend.hpp"
#include "sim/BlochObserver.hpp"

#include <memory>

int main() {
    // Build a 2-qubit circuit: H(0), CNOT(0,1)
    Circuit c;
    c.add({ OpType::H,    {0},   {} });
    c.add({ OpType::CNOT, {0,1}, {} });

    auto backend = std::make_shared<StatevectorBackend>(2, 1);
    Stepper stepper(c, backend);

    // Track Bloch vector of qubit 0 (try also qubit 1)
    auto bloch0 = std::make_shared<BlochObserver>(backend, 0);
    stepper.add_observer(bloch0);

    while (!stepper.done()) stepper.step();
    return 0;
}
