#pragma once

#include "backend/IBackend.hpp"
#include <cstdint>
#include <complex>
#include <vector>
#include <array>

using Complex = std::complex<double>;
using Rho2 = std::array<std::array<Complex,2>,2>;

class IBackendEx : public IBackend {
public:
    virtual ~IBackendEx() = default;

    virtual std::uint32_t num_qubits() const = 0;

    // execution helpers
    virtual void reset() = 0;
    virtual void set_seed(std::uint64_t seed) = 0;
    virtual const std::vector<int>& last_measurements() const = 0;

    // analytics: reduced 1-qubit density matrix for qubit q
    virtual Rho2 reduced_density_1q(std::uint32_t q) const = 0;

    // optional: statevector-only phase support (default: not supported)
    virtual bool try_get_amplitude(std::size_t /*index*/, Complex& /*out*/) const { return false; }
};
