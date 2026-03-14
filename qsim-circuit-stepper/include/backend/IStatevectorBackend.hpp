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

    // Old API (copy) - keep for tests/tools if you want
    virtual std::vector<std::complex<double>> amplitudes() const = 0;

    // New API (NO COPY) - observers should use this
    virtual const std::vector<std::complex<double>>& amplitudes_ref() const = 0;

    virtual std::uint32_t num_qubits() const = 0;

    virtual void reset() = 0;
    virtual void set_seed(std::uint64_t seed) = 0;
    virtual const std::vector<int>& last_measurements() const = 0;
};
