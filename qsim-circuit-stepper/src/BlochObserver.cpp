#include "sim/BlochObserver.hpp"
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>

BlochObserver::BlochObserver(std::shared_ptr<IStatevectorBackend> b)
    : backend(std::move(b)) {}

void BlochObserver::after_step(std::size_t pc, const Instruction& /*instr*/) {
    if (!backend || backend->num_qubits() != 1) return;

    auto amps = backend->amplitudes();
    if (amps.size() != 2) return;

    const std::complex<double> a = amps[0];
    const std::complex<double> b = amps[1];

    const std::complex<double> ab = std::conj(a) * b;

    const double x = 2.0 * ab.real();
    const double y = 2.0 * ab.imag();
    const double z = std::norm(a) - std::norm(b);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "step " << pc
              << " | Bloch (x,y,z)=("
              << x << ", " << y << ", " << z << ")\n";
}
