#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

struct CliOptions {
    std::string demo = "bell";

    std::uint32_t qubits = 2;
    bool qubits_set = false;

    std::string file_path = "";
    std::string qasm_path = "";

    std::uint32_t shots = 0;

    bool has_seed = false;
    std::uint64_t seed = 0;

    bool enable_bloch = true;
    std::uint32_t bloch_qubit = 0;

    bool enable_phase = true;
    std::size_t phase_i = 0;
    std::size_t phase_j = 3;

    bool enable_metrics = true;
    std::uint32_t metrics_qubit = 0;

    std::string trace_path = "";

    bool break_on_op = false;
    std::string break_op = "";

    bool break_on_step = false;
    std::size_t break_step = 0;
// G4
bool dump_state = false;              // statevector amplitudes (small N)
bool dump_rho = false;                // reduced density matrix for one qubit
std::uint32_t dump_rho_qubit = 0;
    // E8
    std::string backend = "statevector"; // statevector | density
    double depolarize = 0.0;
    double dephase = 0.0;
    double amp_damp = 0.0;

    // G1
    bool list_demos = false;
    bool print_circuit = false;
};

CliOptions parse_cli(int argc, char** argv);
