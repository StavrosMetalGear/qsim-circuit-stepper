#include "circuit/Circuit.hpp"
#include "backend/StatevectorBackend.hpp"
#include "sim/ShotsRunner.hpp"

#include <iostream>
#include <memory>

int main() {
    Circuit c;

    // 2-qubit Bell preparation: (|00> + |11>)/sqrt(2)
    c.add({ OpType::H,    {0},   {} });
    c.add({ OpType::CNOT, {0,1}, {} });

    // Measure both qubits
    c.add({ OpType::MEASURE, {0}, {} });
    c.add({ OpType::MEASURE, {1}, {} });

    auto backend = std::make_shared<StatevectorBackend>(2, 1);
    ShotsRunner shots(c, backend);

    const std::uint32_t N = 1000;
    const std::uint64_t seed = 12345;

    auto hist = shots.run(N, seed);

    std::cout << "Shots: " << N << " seed=" << seed << "\n";
    for (const auto& [bits, count] : hist) {
        std::cout << bits << " : " << count << "\n";
    }

    return 0;
}
