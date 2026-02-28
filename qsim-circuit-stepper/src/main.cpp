#include "circuit/Circuit.hpp"
#include "sim/stepper.hpp"
#include "backend/StatevectorBackend.hpp"

#include "sim/BlochObserver.hpp"
#include "sim/PhaseObserver.hpp"
#include "sim/TraceObserver.hpp"

#include <memory>
#include <iostream>

int main() {
    Circuit c;

    // Bell prep: H(0), CNOT(0,1)
    c.add({ OpType::H,    {0},   {} });
    c.add({ OpType::CNOT, {0,1}, {} });

    // Measure both qubits
    c.add({ OpType::MEASURE, {0}, {} });
    c.add({ OpType::MEASURE, {1}, {} });

    auto backend = std::make_shared<StatevectorBackend>(2, 1);
    backend->set_seed(12345); // deterministic measurement during this run

    Stepper stepper(c, backend);

    // Optional console observers (keep if you like)
    stepper.add_observer(std::make_shared<BlochObserver>(backend, 0));
    stepper.add_observer(std::make_shared<PhaseObserver>(backend, 0, 3));

    // Step 6: CSV trace (writes trace.csv in repo root when you run from root build)
    stepper.add_observer(std::make_shared<TraceObserver>(
        backend,
        "trace.csv",
        /*bloch_qubit*/ 0,
        /*phase_i*/ 0,
        /*phase_j*/ 3
    ));

    while (!stepper.done()) stepper.step();

    std::cout << "Wrote trace.csv\n";
    return 0;
}
