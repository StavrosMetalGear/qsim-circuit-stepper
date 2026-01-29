cmake_minimum_required(VERSION 3.20)
project(qsim_circuit_stepper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Submodule paths ----
set(TINY_SIM_DIR "${CMAKE_SOURCE_DIR}/vendor/tiny-1q-sim")

# Fail early if submodule isn't initialized
if(NOT EXISTS "${TINY_SIM_DIR}/include/statevector.hpp")
  message(FATAL_ERROR
    "Submodule not initialized. Run:\n"
    "  git submodule update --init --recursive"
  )
endif()

# ---- Build the tiny simulator as a library (without editing its repo) ----
file(GLOB_RECURSE TINY_SIM_SOURCES CONFIGURE_DEPENDS
  "${TINY_SIM_DIR}/src/*.cpp"
)

add_library(tiny_1q_sim STATIC ${TINY_SIM_SOURCES})

target_include_directories(tiny_1q_sim PUBLIC
  "${TINY_SIM_DIR}/include"
)

# ---- Build your circuit-stepper executable ----
file(GLOB_RECURSE STEPPER_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(qsim_stepper ${STEPPER_SOURCES})

target_include_directories(qsim_stepper PRIVATE
  "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(qsim_stepper PRIVATE tiny_1q_sim)
