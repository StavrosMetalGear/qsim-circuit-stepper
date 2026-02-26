#include "circuit/Circuit.hpp"
#include "sim/stepper.hpp"
#include "backend/StatevectorBackend.hpp"
#include "sim/BlochObserver.hpp"
#include "sim/PhaseObserver.hpp"

#include <memory>

int main() {
    // Bell prep: H(0), CNOT(0,1)
    Circuit c;
    c.add({ OpType::H,    {0},   {} });
    c.add({ OpType::CNOT, {0,1}, {} });

    auto backend = std::make_shared<StatevectorBackend>(2, 1);
    Stepper stepper(c, backend);

    // Bloch vector of qubit 0
    stepper.add_observer(std::make_shared<BlochObserver>(backend, 0));

    // Phase between basis indices: 0=|00>, 3=|11>
    stepper.add_observer(std::make_shared<PhaseObserver>(backend, 0, 3));

    while (!stepper.done()) stepper.step();
    return 0;
}
