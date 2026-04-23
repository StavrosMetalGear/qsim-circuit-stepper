#include "sim/QasmParser.hpp"
#include "circuit/Instruction.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <vector>

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

// split by delimiter (no regex)
static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == delim) {
            out.push_back(trim(cur));
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(trim(cur));
    return out;
}

// q[i] -> i
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

// ---- E7.2: parse common pi expressions ----
// supports: pi, -pi, pi/2, 3*pi/4, -3*pi/4, 0.5*pi, -0.5*pi, 1.234
static double parse_pi_expr(std::string expr) {
    expr = trim(expr);
    if (expr.empty()) throw std::runtime_error("Empty angle expression");

    // remove spaces inside
    expr.erase(std::remove_if(expr.begin(), expr.end(), [](unsigned char c){ return std::isspace(c); }), expr.end());

    // If plain number
    auto has_pi = (expr.find("pi") != std::string::npos);
    if (!has_pi) return std::stod(expr);

    // Handle leading sign
    double sign = 1.0;
    if (!expr.empty() && (expr[0] == '+' || expr[0] == '-')) {
        if (expr[0] == '-') sign = -1.0;
        expr = expr.substr(1);
    }

    // Now expr contains pi somewhere. We accept forms:
    //  "pi"
    //  "pi/2"
    //  "3*pi"
    //  "3*pi/4"
    //  "0.5*pi"
    //  "pi*3" (also)
    // We'll parse numerator and denominator as doubles.

    double num = 1.0;
    double den = 1.0;

    // split by '/'
    auto frac = split(expr, '/');
    if (frac.size() > 2) throw std::runtime_error("Unsupported angle expression: " + expr);

    std::string left = frac[0];
    std::string right = (frac.size() == 2) ? frac[1] : "";

    // parse denominator
    if (!right.empty()) den = std::stod(right);

    // parse left side possibly containing '*'
    auto mul = split(left, '*');
    if (mul.size() == 1) {
        // could be "pi" or "2pi" (we won't support 2pi without '*')
        if (mul[0] == "pi") num = 1.0;
        else throw std::runtime_error("Unsupported angle expression (use '*' with pi): " + left);
    } else if (mul.size() == 2) {
        // one token must be pi
        if (mul[0] == "pi") num = std::stod(mul[1]);
        else if (mul[1] == "pi") num = std::stod(mul[0]);
        else throw std::runtime_error("Angle expression missing pi: " + left);
    } else {
        throw std::runtime_error("Unsupported angle expression: " + left);
    }

    return sign * (num * M_PI / den);
}

static double parse_angle_in_parens(const std::string& op) {
    auto l = op.find('(');
    auto r = op.find(')');
    if (l == std::string::npos || r == std::string::npos || r <= l + 1)
        throw std::runtime_error("Bad rotation op: " + op);
    return parse_pi_expr(op.substr(l + 1, r - (l + 1)));
}

ParsedQasm QasmParser::parse_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Could not open QASM file: " + path);

    ParsedQasm out;
    std::string qreg_name = "q";

    std::string line;
    std::size_t lineno = 0;

    auto add_1q = [&](OpType t, int q, double param = 0.0, bool has_param = false) {
        Instruction instr;
        instr.type = t;
        instr.targets = { q };
        if (has_param) instr.params = { param };
        out.circuit.add(instr);
    };

    auto add_cnot = [&](int c, int t) {
        Instruction instr;
        instr.type = OpType::CNOT;
        instr.targets = { c, t };
        out.circuit.add(instr);
    };

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

        // creg ignored
        if (starts_with(line, "creg")) continue;

        // measure q[i] -> c[j]
        if (starts_with(line, "measure")) {
            std::istringstream iss(line);
            std::string kw, qt, arrow, ct;
            iss >> kw >> qt >> arrow >> ct;
            if (qt.empty()) throw std::runtime_error(path + ":" + std::to_string(lineno) + " bad measure");

            int q = parse_q_index(qt, qreg_name);
            add_1q(OpType::MEASURE, q);
            continue;
        }

        // op ...
        std::istringstream iss(line);
        std::string op;
        iss >> op;
        if (op.empty()) continue;

        // ---- E7.2: U gate family ----
        // u3(theta,phi,lambda) q[i] => RZ(phi) RY(theta) RZ(lambda)
        // u2(phi,lambda) q[i] => RZ(phi) RY(pi/2) RZ(lambda)
        // u1(lambda) q[i] => RZ(lambda)
        if (starts_with(op, "u3(") || starts_with(op, "u2(") || starts_with(op, "u1(")) {
            std::string arg;
            iss >> arg;
            if (arg.empty()) throw std::runtime_error(path + ":" + std::to_string(lineno) + " missing target");

            int q = parse_q_index(arg, qreg_name);

            auto l = op.find('(');
            auto r = op.find(')');
            if (l == std::string::npos || r == std::string::npos || r <= l + 1)
                throw std::runtime_error(path + ":" + std::to_string(lineno) + " bad u*()");

            auto inside = op.substr(l + 1, r - (l + 1));

            auto args = split(inside, ',');

            if (starts_with(op, "u3(")) {
                if (args.size() != 3) throw std::runtime_error("u3 expects 3 args");
                double theta = parse_pi_expr(args[0]);
                double phi   = parse_pi_expr(args[1]);
                double lam   = parse_pi_expr(args[2]);
                add_1q(OpType::RZ, q, phi, true);
                add_1q(OpType::RY, q, theta, true);
                add_1q(OpType::RZ, q, lam, true);
            } else if (starts_with(op, "u2(")) {
                if (args.size() != 2) throw std::runtime_error("u2 expects 2 args");
                double phi = parse_pi_expr(args[0]);
                double lam = parse_pi_expr(args[1]);
                add_1q(OpType::RZ, q, phi, true);
                add_1q(OpType::RY, q, (M_PI / 2.0), true);
                add_1q(OpType::RZ, q, lam, true);
            } else { // u1
                if (args.size() != 1) throw std::runtime_error("u1 expects 1 arg");
                double lam = parse_pi_expr(args[0]);
                add_1q(OpType::RZ, q, lam, true);
            }
            continue;
        }

        // ---- rotations: rx(θ)/ry(θ)/rz(θ) q[i] ----
        if (starts_with(op, "rx(") || starts_with(op, "ry(") || starts_with(op, "rz(")) {
            std::string arg;
            iss >> arg;
            if (arg.empty()) throw std::runtime_error(path + ":" + std::to_string(lineno) + " missing target");

            const double theta = parse_angle_in_parens(op);
            int q = parse_q_index(arg, qreg_name);

            if (starts_with(op, "rx(")) add_1q(OpType::RX, q, theta, true);
            else if (starts_with(op, "ry(")) add_1q(OpType::RY, q, theta, true);
            else add_1q(OpType::RZ, q, theta, true);

            continue;
        }

        // ---- 2-qubit gates ----
        // cx q[a],q[b]
        // cz q[a],q[b]  (decompose: H(target), CNOT, H(target))
        // swap q[a],q[b] (decompose: CNOT a b; CNOT b a; CNOT a b)
        if (op == "cx" || op == "cz" || op == "swap") {
            std::string rest;
            std::getline(iss, rest);
            rest = trim(rest);
            auto comma = rest.find(',');
            if (comma == std::string::npos)
                throw std::runtime_error(path + ":" + std::to_string(lineno) + " bad 2q args");

            std::string left = trim(rest.substr(0, comma));
            std::string right = trim(rest.substr(comma + 1));

            int a = parse_q_index(left, qreg_name);
            int b = parse_q_index(right, qreg_name);

            if (op == "cx") {
                add_cnot(a, b);
            } else if (op == "cz") {
                // CZ(control=a, target=b) via H(b) CNOT(a,b) H(b)
                add_1q(OpType::H, b);
                add_cnot(a, b);
                add_1q(OpType::H, b);
            } else { // swap
                add_cnot(a, b);
                add_cnot(b, a);
                add_cnot(a, b);
            }
            continue;
        }

        // ---- 1-qubit gates ----
        // h/x/y/z q[i]
        // id q[i] (ignore)
        std::string arg;
        iss >> arg;
        if (arg.empty()) throw std::runtime_error(path + ":" + std::to_string(lineno) + " missing target");

        if (op == "id") {
            continue; // no-op
        }

        int q = parse_q_index(arg, qreg_name);

        if (op == "h") add_1q(OpType::H, q);
        else if (op == "x") add_1q(OpType::X, q);
        else if (op == "y") add_1q(OpType::Y, q);
        else if (op == "z") add_1q(OpType::Z, q);
        else {
            throw std::runtime_error(path + ":" + std::to_string(lineno) + " unsupported op '" + op + "'");
        }
    }

    if (out.qubits == 0) throw std::runtime_error("QASM missing qreg: " + path);
    return out;
}
