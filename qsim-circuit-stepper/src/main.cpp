#include "circuit/Circuit.hpp"
#include "sim/stepper.hpp"
#include "backend/StatevectorBackend.hpp"
#include "sim/BlochObserver.hpp"

#include <memory>

int main() {
    Circuit c;

    // Rotate around X then Z then Y (angles in radians)
    c.add({ OpType::RX, {0}, {1.0} });
    c.add({ OpType::RZ, {0}, {0.7} });
    c.add({ OpType::RY, {0}, {1.2} });

    auto backend = std::make_shared<StatevectorBackend>(1, 1);
    Stepper stepper(c, backend);

    stepper.add_observer(std::make_shared<BlochObserver>(backend, 0));

    while (!stepper.done()) stepper.step();
    return 0;
}
