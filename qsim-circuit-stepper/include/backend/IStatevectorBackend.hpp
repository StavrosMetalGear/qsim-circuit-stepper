#pragma once
#include "backend/IBackend.hpp"

// Forward declare your statevector type (we will replace this name to match Repo #1)
class Statevector;

class IStatevectorBackend : public IBackend {
public:
    virtual ~IStatevectorBackend() = default;

    // Read-only access for observers (probabilities, Bloch, etc.)
    virtual const Statevector& state() const = 0;

    // Optional: number of qubits
    virtual int num_qubits() const = 0;
};

