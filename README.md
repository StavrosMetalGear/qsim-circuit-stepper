# qsim-circuit-stepper

A C++ circuit “stepper” simulator skeleton with debugger-style hooks .
This repository focuses on *circuit execution + stepping + observability*, and is designed to use a separate core math/statevector library.

## Goals
- Represent quantum circuits as an instruction list
- Execute circuits step-by-step (program counter / `pc`)
- Provide observer hooks for debugging, tracing, and visualization

## Project layout
- `include/circuit/` : circuit IR (Instruction, Circuit, Gate types)
- `include/sim/`     : Stepper + Observer interfaces (debug hooks)
- `include/backend/` : execution backend interface / implementations (statevector later)
- `src/`             : implementations + demo `main.cpp`
- `vendor/`          : external or sibling core library (optional)

## Build
Open the Visual Studio solution and build/run the Console App.

## Roadmap
- Add `StatevectorBackend` that applies operations to a statevector
- Add Observers (probabilities, Bloch vector, trace logging)
- Add breakpoints and “run until” controls
- Add measurement sampling (shots + seeded RNG)

## Build (CMake)

Clone with submodules:

```bash
git clone --recurse-submodules git@github.com:StavrosMetalGear/qsim-circuit-stepper.git
cd qsim-circuit-stepper

git submodule update --init --recursive
cmake -S . -B build
cmake --build build -j
./build/qsim_stepper --demo bell
./build/qsim_stepper --demo rot1q --trace trace.csv
./build/qsim_stepper --demo bell --shots 1000 --seed 7
./build/qsim_stepper --demo bell --break-on MEASURE
H 0
CNOT 0 1
MEASURE 0
MEASURE 1
./build/qsim_stepper --file circuits/bell.qc --shots 2000 --seed 7
./build/qsim_stepper --file circuits/bell.qc --trace trace.csv
ctest --test-dir build --output-on-failure

### 2) Commit E4
```bash
git add README.md
git commit -m "Phase E4: document build, CLI usage, and tests"
git push
mkdir -p .github/workflows

cat > .github/workflows/ci.yml <<'EOF'
name: CI

on:
  push:
  pull_request:

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout (with submodules)
        uses: actions/checkout@v4
        with:
          submodules: recursive

      - name: Configure

        run: cmake -S . -B build

      - name: Build
        run: cmake --build build -j 2

      - name: Test
        run: ctest --test-dir build --output-on-failure

