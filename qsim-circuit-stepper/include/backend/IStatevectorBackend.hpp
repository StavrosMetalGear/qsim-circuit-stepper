#pragma once
#include "backend/IBackend.hpp"
#include <cstdint>

namespace qsim { class StateVector; }

class IStatevectorBackend : public IBackend {
public:
    virtual ~IStatevectorBackend() = default;
    virtual const qsim::StateVector& state() const = 0;
    virtual std::uint32_t num_qubits() const = 0;
};
