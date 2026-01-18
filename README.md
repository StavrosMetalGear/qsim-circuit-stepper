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
