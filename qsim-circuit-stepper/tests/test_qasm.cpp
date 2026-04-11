#include "sim/QasmParser.hpp"
#include "circuit/Instruction.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

static void expect_true(bool cond, const char* msg) {
    if (!cond) throw std::runtime_error(msg);
}

static void expect_eq_u32(std::uint32_t a, std::uint32_t b, const char* msg) {
    if (a != b) {
        std::cerr << msg << " got=" << a << " expected=" << b << "\n";
        throw std::runtime_error(msg);
    }
}

static void expect_eq_i(int a, int b, const char* msg) {
    if (a != b) {
        std::cerr << msg << " got=" << a << " expected=" << b << "\n";
        throw std::runtime_error(msg);
    }
}

static void expect_parse_ok(const std::string& path) {
    ParsedQasm p = QasmParser::parse_file(path);
    expect_true(p.qubits > 0, "QASM: expected qubits > 0");
    expect_true(p.circuit.size() > 0, "QASM: expected non-empty circuit");
}

void run_qasm_tests() {
    std::cout << "[F1.1] qasm parser tests...\n";

    // 1) Basic bell.qasm should parse, 2 qubits, and contain H, CNOT, MEASURE...
    {
        ParsedQasm p = QasmParser::parse_file("circuits/qasm/bell.qasm");
        expect_eq_u32(p.qubits, 2, "bell.qasm should declare 2 qubits");
        expect_true(p.circuit.size() >= 4, "bell.qasm should have >= 4 instructions");

        const auto& i0 = p.circuit[0];
        expect_true(i0.type == OpType::H, "bell.qasm: first op should be H");
        expect_true(!i0.targets.empty(), "bell.qasm: H should have a target");
        expect_eq_i(i0.targets[0], 0, "bell.qasm: H should target q[0]");

        bool has_cnot = false;
        bool has_measure = false;
        for (std::size_t k = 0; k < p.circuit.size(); ++k) {
            if (p.circuit[k].type == OpType::CNOT) has_cnot = true;
            if (p.circuit[k].type == OpType::MEASURE) has_measure = true;
        }
        expect_true(has_cnot, "bell.qasm should contain CNOT");
        expect_true(has_measure, "bell.qasm should contain MEASURE");
    }

    // 2) E7.2 demo files should parse (u3/pi, cz, swap)
    expect_parse_ok("circuits/qasm/rot1q.qasm");
    expect_parse_ok("circuits/qasm/u3_pi_demo.qasm");
    expect_parse_ok("circuits/qasm/cz_demo.qasm");
    expect_parse_ok("circuits/qasm/swap_demo.qasm");

    // 3) Negative test: missing file should throw
    {
        bool threw = false;
        try {
            (void)QasmParser::parse_file("circuits/qasm/this_file_should_not_exist.qasm");
        } catch (...) {
            threw = true;
        }
        expect_true(threw, "parse_file should throw on missing file");
    }

    std::cout << "[F1.1] OK\n";
}
