#include "sim/ShotsRunner.hpp"
#include "sim/stepper.hpp"

#include <algorithm>

ShotsRunner::ShotsRunner(const Circuit& circuit, std::shared_ptr<IStatevectorBackend> backend)
    : m_circuit(circuit), m_backend(std::move(backend)) {}

std::string ShotsRunner::readout_bitstring() const {
    const auto& meas = m_backend->last_measurements();
    const std::uint32_t n = m_backend->num_qubits();

    std::string s;
    s.reserve(n);

    // Common convention: highest qubit on left, qubit 0 on right
    for (int q = (int)n - 1; q >= 0; --q) {
        int v = (q < (int)meas.size()) ? meas[q] : -1;
        s.push_back(v == 1 ? '1' : '0'); // treat -1 as 0 if not measured
    }
    return s;
}

std::map<std::string, std::uint64_t> ShotsRunner::run(std::uint32_t shots, std::uint64_t seed) {
    std::map<std::string, std::uint64_t> hist;

    // deterministic: seed once, then measurements consume RNG in a repeatable way
    m_backend->set_seed(seed);

    for (std::uint32_t k = 0; k < shots; ++k) {
        m_backend->reset();

        // run the circuit from scratch
        Stepper s(m_circuit, m_backend);
        while (!s.done()) s.step();

        // collect readout (assumes circuit includes MEASURE ops)
        hist[readout_bitstring()]++;
    }

    return hist;
}
