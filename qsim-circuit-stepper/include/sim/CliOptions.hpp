#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

struct CliOptions {
    std::string demo = "bell";        // bell | rot1q
    std::uint32_t qubits = 2;

    // execution mode
    std::uint32_t shots = 0;          // 0 => step mode, >0 => shots mode

    // deterministic
    bool has_seed = false;
    std::uint64_t seed = 0;

    // observers
    bool enable_bloch = true;
    std::uint32_t bloch_qubit = 0;

    bool enable_phase = true;
    std::size_t phase_i = 0;
    std::size_t phase_j = 3;          // for bell defaults

    // trace
    std::string trace_path = "";      // empty => no trace

    // breakpoints (step mode only)
    bool break_on_op = false;
    std::string break_op = "";        // e.g. "MEASURE"

    bool break_on_step = false;
    std::size_t break_step = 0;
};

CliOptions parse_cli(int argc, char** argv);
