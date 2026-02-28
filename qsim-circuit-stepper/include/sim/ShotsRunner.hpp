#pragma once

#include "circuit/Circuit.hpp"
#include "backend/IStatevectorBackend.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <string>

class ShotsRunner {
public:
    ShotsRunner(const Circuit& circuit, std::shared_ptr<IStatevectorBackend> backend);

    // Run N shots, deterministic with seed. Returns histogram: bitstring -> count.
    std::map<std::string, std::uint64_t> run(std::uint32_t shots, std::uint64_t seed);

private:
    const Circuit& m_circuit;
    std::shared_ptr<IStatevectorBackend> m_backend;

    std::string readout_bitstring() const; // qubit0 is rightmost by default
};
