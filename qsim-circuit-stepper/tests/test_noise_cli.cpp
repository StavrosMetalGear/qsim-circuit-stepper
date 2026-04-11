#include "backend/DensityMatrixBackend.hpp"
#include "sim/ShotsRunner.hpp"
#include "circuit/Circuit.hpp"
#include "circuit/Instruction.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <cstdint>

static void expect_true(bool cond, const char* msg) {
    if (!cond) throw std::runtime_error(msg);
}

static Circuit make_bell_2q_with_meas() {
    Circuit c;
    c.add({OpType::H, {0}, {}});
    c.add({OpType::CNOT, {0,1}, {}});
    c.add({OpType::MEASURE, {0}, {}});
    c.add({OpType::MEASURE, {1}, {}});
    return c;
}

static double prob_of(const std::map<std::string, std::uint64_t>& hist,
                      const std::string& bits,
                      std::uint32_t shots)
{
    auto it = hist.find(bits);
    if (it == hist.end()) return 0.0;
    return double(it->second) / double(shots);
}

void run_noise_sanity_tests() {
    std::cout << "[F1.2] noise sanity tests...\n";

    const auto bell = make_bell_2q_with_meas();
    const std::uint32_t shots = 600;
    const std::uint64_t seed = 7;

    // 1) Dephasing should reduce Bell correlations: (p00+p11) drops when dephase increases.
    {
        auto low = std::make_shared<DensityMatrixBackend>(2, /*depol*/0.0, /*dephase*/0.00, /*amp*/0.0);
        auto high = std::make_shared<DensityMatrixBackend>(2, /*depol*/0.0, /*dephase*/0.20, /*amp*/0.0);

        ShotsRunner sr_low(bell, low);
        ShotsRunner sr_high(bell, high);

        auto h_low = sr_low.run(shots, seed);
        auto h_high = sr_high.run(shots, seed);

        const double corr_low  = prob_of(h_low, "00", shots) + prob_of(h_low, "11", shots);
        const double corr_high = prob_of(h_high, "00", shots) + prob_of(h_high, "11", shots);

        std::cout << "  dephase 0.00 corr=" << corr_low << "\n";
        std::cout << "  dephase 0.20 corr=" << corr_high << "\n";

        expect_true(corr_low > corr_high, "Expected Bell correlation to drop as dephase increases");
    }

    // 2) Amplitude damping should reduce the probability of measuring 11 compared to no amp damping.
    {
        auto noamp = std::make_shared<DensityMatrixBackend>(2, /*depol*/0.0, /*dephase*/0.0, /*amp*/0.0);
        auto amp   = std::make_shared<DensityMatrixBackend>(2, /*depol*/0.0, /*dephase*/0.0, /*amp*/0.20);

        ShotsRunner sr_noamp(bell, noamp);
        ShotsRunner sr_amp(bell, amp);

        auto h0 = sr_noamp.run(shots, seed);
        auto h1 = sr_amp.run(shots, seed);

        const double p11_noamp = prob_of(h0, "11", shots);
        const double p11_amp   = prob_of(h1, "11", shots);

        std::cout << "  amp 0.00 p11=" << p11_noamp << "\n";
        std::cout << "  amp 0.20 p11=" << p11_amp << "\n";

        expect_true(p11_noamp > p11_amp, "Expected P(11) to drop with amplitude damping");
    }

    std::cout << "[F1.2] OK\n";
}
