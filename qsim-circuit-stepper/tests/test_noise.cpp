#include "backend/DensityMatrixBackend.hpp"
#include "circuit/Instruction.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

static void expect_true(bool cond, const char* msg) {
    if (!cond) throw std::runtime_error(msg);
}

static void expect_near(double a, double b, double eps, const char* msg) {
    if (std::fabs(a - b) > eps) {
        std::cerr << msg << " got=" << a << " expected=" << b << " eps=" << eps << "\n";
        throw std::runtime_error(msg);
    }
}

static double purity_1q(const Rho2& r) {
    const double a = r[0][0].real();
    const double d = r[1][1].real();
    const double off2 = std::norm(r[0][1]);
    return a*a + d*d + 2.0*off2;
}

static double coherence_1q(const Rho2& r) {
    return 2.0 * std::abs(r[0][1]);
}

static Instruction make_1q(OpType t, int q, double param0 = 0.0, bool use_param = false) {
    Instruction ins;
    ins.type = t;
    ins.targets = { q };
    if (use_param) ins.params = { param0 };
    return ins;
}

// We use RZ(0) as a “do nothing” gate that still triggers noise application.
static void noise_tick(DensityMatrixBackend& dm, int q = 0) {
    dm.apply(make_1q(OpType::RZ, q, 0.0, true));
}

void run_noise_tests() {
    std::cout << "[E8.5] noise tests...\n";

    // 1) Trace(reduced rho) == 1 always
    {
        DensityMatrixBackend dm(1, /*depol*/0.0, /*dephase*/0.05, /*amp*/0.02);
        dm.set_seed(7);

        dm.apply(make_1q(OpType::H, 0)); // |+>
        for (int k = 0; k < 20; ++k) noise_tick(dm);

        auto r = dm.reduced_density_1q(0);
        const double tr = r[0][0].real() + r[1][1].real();
        expect_near(tr, 1.0, 1e-9, "reduced trace must be 1");
    }

    // 2) Dephasing reduces coherence of |+> (off-diagonal shrinks)
    {
        DensityMatrixBackend dm(1, /*depol*/0.0, /*dephase*/0.10, /*amp*/0.0);
        dm.set_seed(7);

        dm.apply(make_1q(OpType::H, 0)); // |+>
        auto r0 = dm.reduced_density_1q(0);
        const double c0 = coherence_1q(r0);

        for (int k = 0; k < 10; ++k) noise_tick(dm);

        auto r1 = dm.reduced_density_1q(0);
        const double c1 = coherence_1q(r1);

        expect_true(c1 < c0, "dephasing should reduce coherence");
    }

    // 3) Amplitude damping drives |1> -> |0>
    {
        DensityMatrixBackend dm(1, /*depol*/0.0, /*dephase*/0.0, /*amp*/0.20);
        dm.set_seed(7);

        dm.apply(make_1q(OpType::X, 0)); // start in |1>
        auto r0 = dm.reduced_density_1q(0);
        const double p1_0 = r0[1][1].real();

        for (int k = 0; k < 15; ++k) noise_tick(dm);

        auto r1 = dm.reduced_density_1q(0);
        const double p1_1 = r1[1][1].real();

        expect_true(p1_1 < p1_0, "amp damping should decrease P(|1>)");
        expect_true(p1_1 < 0.5, "amp damping should significantly relax |1> after enough ticks");
    }

    // 4) Depolarizing reduces purity (from near-pure state)
    {
        DensityMatrixBackend dm(1, /*depol*/0.20, /*dephase*/0.0, /*amp*/0.0);
        dm.set_seed(7);

        dm.apply(make_1q(OpType::H, 0)); // pure |+>
        auto r0 = dm.reduced_density_1q(0);
        const double p0 = purity_1q(r0);

        for (int k = 0; k < 10; ++k) noise_tick(dm);

        auto r1 = dm.reduced_density_1q(0);
        const double p1 = purity_1q(r1);

        expect_true(p1 < p0, "depolarizing should reduce purity");
    }

    std::cout << "[E8.5] OK\n";
}
