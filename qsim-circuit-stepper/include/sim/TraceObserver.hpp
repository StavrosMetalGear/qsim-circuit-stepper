#pragma once

#include "sim/Observer.hpp"
#include "backend/IBackendEx.hpp"

#include <fstream>
#include <memory>
#include <string>
#include <cstdint>
#include <cstddef>

class TraceObserver final : public Observer {
public:
    TraceObserver(std::shared_ptr<IBackendEx> backend,
                  const std::string& csv_path,
                  std::uint32_t qubit_for_metrics = 0,
                  std::size_t phase_i = 0,
                  std::size_t phase_j = 1);

    void after_step(std::size_t step, const Instruction& instr) override;

private:
    std::shared_ptr<IBackendEx> m_backend;
    std::ofstream m_out;

    std::uint32_t m_q = 0;
    std::size_t m_i = 0;
    std::size_t m_j = 1;

    bool m_wrote_header = false;
};
