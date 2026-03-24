#pragma once

#include "circuit/Circuit.hpp"
#include <cstdint>
#include <string>

struct ParsedQasm {
    Circuit circuit;
    std::uint32_t qubits = 0;   // from qreg
};

class QasmParser {
public:
    // Parses a small OpenQASM 2.0 subset.
    static ParsedQasm parse_file(const std::string& path);
};
