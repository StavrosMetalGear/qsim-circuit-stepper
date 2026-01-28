#include "circuit/Circuit.hpp"
#include "sim/Stepper.hpp"
#include "backend/StatevectorBackend.hpp"
#include <memory>

int main() {
    Circuit c;
    c.add({ OpType::H, {0}, {} });
    c.add({ OpType::RZ, {0}, {1.57079632679} });

    auto backend = std::make_shared<StatevectorBackend>(1,1);
    Stepper stepper(circuit, backend);

    while (!stepper.done()) {
        stepper.step();
    }
    return 0;
}
