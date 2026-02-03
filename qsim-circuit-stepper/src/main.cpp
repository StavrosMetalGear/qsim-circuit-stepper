#include "circuit/Circuit.hpp"
#include "sim/stepper.hpp"
#include "backend/StatevectorBackend.hpp"
#include "sim/BlochObserver.hpp"

#include <memory>

int main() {
    Circuit c;

    // Demo: H -> Z -> H (shows Bloch changes)
    c.add({ OpType::H, {0}, {} });
    c.add({ OpType::Z, {0}, {} });
    c.add({ OpType::H, {0}, {} });

    auto backend = std::make_shared<StatevectorBackend>(1, 1);
    Stepper s(c, backend);

    auto bloch = std::make_shared<BlochObserver>(backend);
    s.add_observer(bloch);

    while (!s.done()) s.step();
    return 0;
}
