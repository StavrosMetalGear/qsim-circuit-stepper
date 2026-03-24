#pragma once

#include "backend/IBackendEx.hpp"
#include <complex>
#include <vector>
#include <cstdint>

namespace qsim { class StateVector; }

class IStatevectorBackend : public IBackendEx {
public:
    virtual ~IStatevectorBackend() = default;

    virtual const qsim::StateVector& state() const = 0;

    // Old API (copy)
    virtual std::vector<std::complex<double>> amplitudes() const = 0;

    // New API (no copy)
    virtual const std::vector<std::complex<double>>& amplitudes_ref() const = 0;
};
