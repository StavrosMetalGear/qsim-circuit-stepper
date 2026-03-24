#include "sim/QasmParser.hpp"
#include "circuit/Instruction.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
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

static void strip_semicolon(std::string& s) {
    s = trim(s);
    if (!s.empty() && s.back() == ';') s.pop_back();
}

static int parse_q_index(const std::string& tok, const std::string& regname) {
    auto l = tok.find('[');
    auto r = tok.find(']');
    if (l == std::string::npos || r == std::string::npos || r <= l + 1)
        throw std::runtime_error("Bad qubit token: " + tok);
    std::string name = tok.substr(0, l);
    if (name != regname)
        throw std::runtime_error("Expected register '" + regname + "' but got '" + name + "'");
    return std::stoi(tok.substr(l + 1, r - (l + 1)));
}

static double parse_angle_in_parens(const std::string& op) {
    auto l = op.find('(');
    auto r = op.find(')');
    if (l == std::string::npos || r == std::string::npos || r <= l + 1)
        throw std::runtime_error("Bad rotation op: " + op);
    return std::stod(op.substr(l + 1, r - (l + 1)));
}

ParsedQasm QasmParser::parse_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Could not open QASM file: " + path);

    ParsedQasm out;
    std::string qreg_name = "q";

    std::string line;
    std::size_t lineno = 0;

    while (std::getline(in, line)) {
        lineno++;
        line = trim(line);
        if (line.empty()) continue;

        // strip inline comments
        auto cpos = line.find("//");
        if (cpos != std::string::npos) line = trim(line.substr(0, cpos));
        if (line.empty()) continue;

        strip_semicolon(line);
        if (line.empty()) continue;

        // ignore boilerplate
        if (starts_with(line, "OPENQASM")) continue;
        if (starts_with(line, "include")) continue;
        if (starts_with(line, "barrier")) continue;

        // qreg q[2]
        if (starts_with(line, "qreg")) {
            std::istringstream iss(line);
            std::string kw, regtok;
            iss >> kw >> regtok;

            auto l = regtok.find('[');
            auto r = regtok.find(']');
            if (l == std::string::npos || r == std::string::npos || r <= l + 1)
                throw std::runtime_error(path + ":" + std::to_string(lineno) + " bad qreg");

            qreg_name = regtok.substr(0, l);
            out.qubits = (std::uint32_t)std::stoul(regtok.substr(l + 1, r - (l + 1)));
            continue;
        }

        // creg c[2] (ignored)
        if (starts_with(line, "creg")) continue;

        // measure q[i] -> c[j]
        if (starts_with(line, "measure")) {
            std::istringstream iss(line);
            std::string kw, qt, arrow, ct;
            iss >> kw >> qt >> arrow >> ct;
            if (qt.empty()) throw std::runtime_error(path + ":" + std::to_string(lineno) + " bad measure");

            int q = parse_q_index(qt, qreg_name);

            Instruction instr;
            instr.type = OpType::MEASURE;
            instr.targets = { q };
            out.circuit.add(instr);
            continue;
        }

        // op args...
        std::istringstream iss(line);
        std::string op;
        iss >> op;
        if (op.empty()) continue;

        // rotations: rx(theta) q[i]
        if (starts_with(op, "rx(") || starts_with(op, "ry(") || starts_with(op, "rz(")) {
            std::string arg;
            iss >> arg;
            if (arg.empty()) throw std::runtime_error(path + ":" + std::to_string(lineno) + " missing target");

            const double theta = parse_angle_in_parens(op);

            Instruction instr;
            if (starts_with(op, "rx(")) instr.type = OpType::RX;
            else if (starts_with(op, "ry(")) instr.type = OpType::RY;
            else instr.type = OpType::RZ;

            int q = parse_q_index(arg, qreg_name);
            instr.targets = { q };
            instr.params = { theta };
            out.circuit.add(instr);
            continue;
        }

        // cx q[a],q[b]
        if (op == "cx") {
            std::string rest;
            std::getline(iss, rest);
            rest = trim(rest);

            auto comma = rest.find(',');
            if (comma == std::string::npos)
                throw std::runtime_error(path + ":" + std::to_string(lineno) + " bad cx args");

            std::string left = trim(rest.substr(0, comma));
            std::string right = trim(rest.substr(comma + 1));

            int c = parse_q_index(left, qreg_name);
            int t = parse_q_index(right, qreg_name);

            Instruction instr;
            instr.type = OpType::CNOT;
            instr.targets = { c, t };
            out.circuit.add(instr);
            continue;
        }

        // 1q ops: h/x/y/z q[i]
        std::string arg;
        iss >> arg;
        if (arg.empty()) throw std::runtime_error(path + ":" + std::to_string(lineno) + " missing target");

        Instruction instr;
        if (op == "h") instr.type = OpType::H;
        else if (op == "x") instr.type = OpType::X;
        else if (op == "y") instr.type = OpType::Y;
        else if (op == "z") instr.type = OpType::Z;
        else throw std::runtime_error(path + ":" + std::to_string(lineno) + " unsupported op '" + op + "'");

        int q = parse_q_index(arg, qreg_name);
        instr.targets = { q };
        out.circuit.add(instr);
    }

    if (out.qubits == 0) throw std::runtime_error("QASM file missing qreg declaration: " + path);
    return out;
}
