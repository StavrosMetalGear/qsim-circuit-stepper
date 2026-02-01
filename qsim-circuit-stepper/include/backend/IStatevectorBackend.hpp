#pragma once
#include "backend/IBackend.hpp"

#include <cstdint>
#include <complex>
#include <vector>

namespace qsim { class StateVector; }

class IStatevectorBackend : public IBackend {
public:
    virtual ~IStatevectorBackend() = default;

    // Access to underlying state object (read-only)
    virtual const qsim::StateVector& state() const = 0;

    // Convenience: return a copy of amplitudes for observers/debug printing
    virtual std::vector<std::complex<double>> amplitudes() const = 0;

    virtual std::uint32_t num_qubits() const = 0;
};
