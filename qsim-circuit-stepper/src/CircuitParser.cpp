#include "sim/CircuitParser.hpp"
#include "circuit/Instruction.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

static std::string trim(std::string s) {
    auto notspace = [](unsigned char c){ return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notspace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notspace).base(), s.end());
    return s;
}

static bool starts_with(const std::string& s, const std::string& p) {
    return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}

static bool parse_op(const std::string& s, OpType& out) {
    if (s == "H") { out = OpType::H; return true; }
    if (s == "X") { out = OpType::X; return true; }
    if (s == "Y") { out = OpType::Y; return true; }
    if (s == "Z") { out = OpType::Z; return true; }
    if (s == "RX") { out = OpType::RX; return true; }
    if (s == "RY") { out = OpType::RY; return true; }
    if (s == "RZ") { out = OpType::RZ; return true; }
    if (s == "CNOT") { out = OpType::CNOT; return true; }
    if (s == "MEASURE") { out = OpType::MEASURE; return true; }
    return false;
}

ParsedCircuit CircuitParser::parse_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Could not open circuit file: " + path);

    ParsedCircuit out;
    std::uint32_t max_q = 0;
    bool any_q = false;

    std::string line;
    std::size_t lineno = 0;

    while (std::getline(in, line)) {
        lineno++;
        line = trim(line);
        if (line.empty()) continue;
        if (starts_with(line, "#") || starts_with(line, "//")) continue;

        std::istringstream iss(line);
        std::string op_s;
        iss >> op_s;

        OpType op;
        if (!parse_op(op_s, op)) {
            throw std::runtime_error("Parse error " + path + ":" + std::to_string(lineno) +
                                     " unknown op '" + op_s + "'");
        }

        Instruction instr;
        instr.type = op;

        auto read_int = [&](int& v) {
            if (!(iss >> v)) {
                throw std::runtime_error("Parse error " + path + ":" + std::to_string(lineno) +
                                         " expected integer");
            }
        };
        auto read_double = [&](double& v) {
            if (!(iss >> v)) {
                throw std::runtime_error("Parse error " + path + ":" + std::to_string(lineno) +
                                         " expected number");
            }
        };

        if (op == OpType::CNOT) {
            int c, t;
            read_int(c); read_int(t);
            instr.targets = {c, t};
            max_q = std::max<std::uint32_t>(max_q, (std::uint32_t)std::max(c, t));
            any_q = true;
        } else if (op == OpType::RX || op == OpType::RY || op == OpType::RZ) {
            int q; double theta;
            read_int(q); read_double(theta);
            instr.targets = {q};
            instr.params = {theta};
            max_q = std::max<std::uint32_t>(max_q, (std::uint32_t)q);
            any_q = true;
        } else {
            int q;
            read_int(q);
            instr.targets = {q};
            max_q = std::max<std::uint32_t>(max_q, (std::uint32_t)q);
            any_q = true;
        }

        std::string extra;
        if (iss >> extra) {
            throw std::runtime_error("Parse error " + path + ":" + std::to_string(lineno) +
                                     " unexpected extra token '" + extra + "'");
        }

        out.circuit.add(instr);
    }

    out.inferred_qubits = any_q ? (max_q + 1) : 0;
    return out;
}
