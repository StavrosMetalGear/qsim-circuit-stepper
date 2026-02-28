#pragma once

#include "backend/IBackend.hpp"
#include <cstdint>
#include <complex>
#include <vector>

namespace qsim { class StateVector; }

class IStatevectorBackend : public IBackend {
public:
    virtual ~IStatevectorBackend() = default;

    virtual const qsim::StateVector& state() const = 0;
    virtual std::vector<std::complex<double>> amplitudes() const = 0;
    virtual std::uint32_t num_qubits() const = 0;

    // Step 5 additions
    virtual void reset() = 0;                    // back to |00..0>
    virtual void set_seed(std::uint64_t seed) = 0; // deterministic measurement
    virtual const std::vector<int>& last_measurements() const = 0; // per-qubit results
};
