#!/usr/bin/env bash
set -euo pipefail

BIN="${1:-./build/qsim_stepper}"

echo "== Build check =="
if [[ ! -x "$BIN" ]]; then
  echo "Binary not found/executable: $BIN"
  echo "Try: cmake -S . -B build && cmake --build build -j"
  exit 1
fi

echo
echo "== Demo 1: Bell (step mode, with metrics) =="
"$BIN" --file circuits/bell.qc

echo
echo "== Demo 2: Bell (shots) =="
"$BIN" --file circuits/bell.qc --shots 500 --seed 7

echo
echo "== Demo 3: Bell (break before MEASURE) =="
"$BIN" --file circuits/bell.qc --break-on MEASURE || true

echo
echo "== Demo 4: 1-qubit rotations (trace export) =="
rm -f trace.csv
"$BIN" --file circuits/rot1q.qc --trace trace.csv
echo "Wrote trace.csv"

echo
echo "== Demo 5: GHZ-3 (shots) =="
"$BIN" --file circuits/ghz3.qc --shots 500 --seed 7

echo
echo "All demos done."
