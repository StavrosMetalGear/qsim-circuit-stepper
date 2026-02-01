cmake_minimum_required(VERSION 3.16)
project(qsim_circuit_stepper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Paths ----
set(STEPPER_DIR "${CMAKE_SOURCE_DIR}/qsim-circuit-stepper")
set(TINY_SIM_DIR "${CMAKE_SOURCE_DIR}/vendor/tiny-1q-sim")

# ---- Build tiny simulator as a static library ----
file(GLOB_RECURSE TINY_SIM_SOURCES CONFIGURE_DEPENDS
  "${TINY_SIM_DIR}/src/*.cpp"
)

add_library(tiny_1q_sim STATIC ${TINY_SIM_SOURCES})
target_include_directories(tiny_1q_sim PUBLIC
  "${TINY_SIM_DIR}/include"
)

# ---- Build the circuit-stepper executable ----
file(GLOB_RECURSE STEPPER_SOURCES CONFIGURE_DEPENDS
  "${STEPPER_DIR}/src/*.cpp"
)

add_executable(qsim_stepper ${STEPPER_SOURCES})
target_include_directories(qsim_stepper PRIVATE
  "${STEPPER_DIR}/include"
)

target_link_libraries(qsim_stepper PRIVATE tiny_1q_sim)
