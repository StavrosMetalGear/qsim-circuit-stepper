#include "circuit/Circuit.hpp"
#include "backend/StatevectorBackend.hpp"
#include "sim/stepper.hpp"
#include "sim/ShotsRunner.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

static bool close_cd(std::complex<double> a, std::complex<double> b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}

static void run_circuit(Circuit& c, std::shared_ptr<StatevectorBackend> backend) {
    backend->reset();
    Stepper s(c, backend);
    while (!s.done()) s.step();
}

static void test_HH_identity() {
    Circuit c;
    c.add({ OpType::H, {0}, {} });
    c.add({ OpType::H, {0}, {} });

    auto backend = std::make_shared<StatevectorBackend>(1, 1);
    run_circuit(c, backend);

    auto amps = backend->amplitudes(); // copy is fine for tests
    assert(amps.size() == 2);
    // should be |0>
    assert(std::norm(amps[0]) > 1.0 - 1e-9);
    assert(std::norm(amps[1]) < 1e-9);
}

static void test_X_flip() {
    Circuit c;
    c.add({ OpType::X, {0}, {} });

    auto backend = std::make_shared<StatevectorBackend>(1, 1);
    run_circuit(c, backend);

    auto amps = backend->amplitudes();
    assert(amps.size() == 2);
    // should be |1>
    assert(std::norm(amps[0]) < 1e-9);
    assert(std::norm(amps[1]) > 1.0 - 1e-9);
}

static void test_bell_shots_no_01_10() {
    Circuit c;
    c.add({ OpType::H, {0}, {} });
    c.add({ OpType::CNOT, {0,1}, {} });
    c.add({ OpType::MEASURE, {0}, {} });
    c.add({ OpType::MEASURE, {1}, {} });

    auto backend = std::make_shared<StatevectorBackend>(2, 1);
    ShotsRunner sr(c, backend);

    auto hist = sr.run(500, 12345);

    // Bell state should NEVER yield 01 or 10 in ideal simulator
    assert(hist["01"] == 0);
    assert(hist["10"] == 0);

    // and should have some 00 and 11
    assert(hist["00"] > 0);
    assert(hist["11"] > 0);
}

void run_noise_tests();

int main() {
    test_HH_identity();
    test_X_flip();
    test_bell_shots_no_01_10();
    std::cout << "All tests passed.\n";
    run_noise_tests();
run_noise_tests();
    return 0;
}
