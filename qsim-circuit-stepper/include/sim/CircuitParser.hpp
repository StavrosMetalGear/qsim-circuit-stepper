#pragma once

#include "circuit/Circuit.hpp"
#include <cstdint>
#include <string>

struct ParsedCircuit {
    Circuit circuit;
    std::uint32_t inferred_qubits = 0;
};

class CircuitParser {
public:
    static ParsedCircuit parse_file(const std::string& path);
};
