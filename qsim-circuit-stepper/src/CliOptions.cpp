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

        if (a == "--demo") {
            opt.demo = need("--demo");
        } else if (a == "--qubits") {
            opt.qubits = (std::uint32_t)std::stoul(need("--qubits"));
        } else if (a == "--shots") {
            opt.shots = (std::uint32_t)std::stoul(need("--shots"));
        } else if (a == "--seed") {
            opt.has_seed = true;
            opt.seed = (std::uint64_t)std::stoull(need("--seed"));
        } else if (a == "--trace") {
            opt.trace_path = need("--trace");
        } else if (a == "--no-bloch") {
            opt.enable_bloch = false;
        } else if (a == "--bloch-qubit") {
            opt.bloch_qubit = (std::uint32_t)std::stoul(need("--bloch-qubit"));
        } else if (a == "--no-phase") {
            opt.enable_phase = false;
        } else if (a == "--phase") {
            opt.phase_i = (std::size_t)std::stoull(need("--phase"));
            opt.phase_j = (std::size_t)std::stoull(need("--phase"));
        } else if (a == "--break-on") {
            opt.break_on_op = true;
            opt.break_op = need("--break-on");
        } else if (a == "--break-step") {
            opt.break_on_step = true;
            opt.break_step = (std::size_t)std::stoull(need("--break-step"));
        } else if (a == "--help") {
            throw std::runtime_error(
                "Usage:\n"
                "  --demo bell|rot1q\n"
                "  --qubits N\n"
                "  --shots N\n"
                "  --seed S\n"
                "  --trace path.csv\n"
                "  --bloch-qubit q | --no-bloch\n"
                "  --phase i j | --no-phase\n"
                "  --break-on OP | --break-step k\n"
            );
        } else if (is_flag(a)) {
            throw std::runtime_error("Unknown flag: " + a);
        }
    }

    // sane defaults
    if (opt.demo == "rot1q") {
        opt.qubits = 1;
        opt.phase_i = 0;
        opt.phase_j = 1;
    } else if (opt.demo == "bell") {
        if (opt.qubits < 2) opt.qubits = 2;
        opt.phase_i = 0;
        opt.phase_j = 3;
    }

    return opt;
}
