#include "sim/CliOptions.hpp"
#include <stdexcept>
#include <string>

static bool is_flag(const std::string& s) { return s.rfind("--", 0) == 0; }

CliOptions parse_cli(int argc, char** argv) {
    CliOptions opt;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];

        auto need = [&](const char* name) -> std::string {
            if (i + 1 >= argc) throw std::runtime_error(std::string("Missing value after ") + name);
            return std::string(argv[++i]);
        };

        if (a == "--file") opt.file_path = need("--file");
        else if (a == "--qasm") opt.qasm_path = need("--qasm");
        else if (a == "--demo") opt.demo = need("--demo");
        else if (a == "--qubits") { opt.qubits = (std::uint32_t)std::stoul(need("--qubits")); opt.qubits_set = true; }
        else if (a == "--shots") opt.shots = (std::uint32_t)std::stoul(need("--shots"));
        else if (a == "--seed") { opt.has_seed = true; opt.seed = (std::uint64_t)std::stoull(need("--seed")); }
        else if (a == "--trace") opt.trace_path = need("--trace");
        else if (a == "--no-bloch") opt.enable_bloch = false;
        else if (a == "--bloch-qubit") opt.bloch_qubit = (std::uint32_t)std::stoul(need("--bloch-qubit"));
        else if (a == "--no-phase") opt.enable_phase = false;
        else if (a == "--phase") { opt.phase_i = (std::size_t)std::stoull(need("--phase")); opt.phase_j = (std::size_t)std::stoull(need("--phase")); }
        else if (a == "--no-metrics") opt.enable_metrics = false;
        else if (a == "--metrics-qubit") opt.metrics_qubit = (std::uint32_t)std::stoul(need("--metrics-qubit"));
        else if (a == "--break-on") { opt.break_on_op = true; opt.break_op = need("--break-on"); }
        else if (a == "--break-step") { opt.break_on_step = true; opt.break_step = (std::size_t)std::stoull(need("--break-step")); }
        else if (a == "--backend") opt.backend = need("--backend");
        else if (a == "--depolarize") opt.depolarize = std::stod(need("--depolarize"));
        else if (a == "--help") {
            throw std::runtime_error(
                "Usage:\n"
                "  --file path.qc | --qasm path.qasm | --demo bell|rot1q\n"
                "  --backend statevector|density\n"
                "  --depolarize p\n"
                "  --qubits N\n"
                "  --shots N\n"
                "  --seed S\n"
                "  --trace path.csv\n"
                "  --bloch-qubit q | --no-bloch\n"
                "  --phase i j | --no-phase\n"
                "  --metrics-qubit q | --no-metrics\n"
                "  --break-on OP | --break-step k\n"
            );
        } else if (is_flag(a)) {
            throw std::runtime_error("Unknown flag: " + a);
        }
    }

    return opt;
}
