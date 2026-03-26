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
echo "== Demo 1: Bell (step mode, metrics) =="
"$BIN" --file circuits/bell.qc

echo
echo "== Demo 2: Bell (shots) =="
"$BIN" --file circuits/bell.qc --shots 500 --seed 7

echo
echo "== Demo 3: Bell (break before MEASURE) =="
"$BIN" --file circuits/bell.qc --break-on MEASURE || true

echo
echo "== Demo 4: 1-qubit rotations (trace export, statevector) =="
rm -f trace_sv.csv
"$BIN" --file circuits/rot1q.qc --trace trace_sv.csv
echo "Wrote trace_sv.csv"

echo
echo "== Demo 5: GHZ-3 (shots) =="
"$BIN" --file circuits/ghz3.qc --shots 500 --seed 7

echo
echo "== Demo 6 (NEW): Bell with noise (density backend) =="
echo "dephase=0.05 amp-damp=0.02 depolarize=0.01"
"$BIN" --qasm circuits/qasm/bell.qasm --backend density --shots 500 --seed 7 \
  --dephase 0.05 --amp-damp 0.02 --depolarize 0.01

echo
echo "== Demo 7 (NEW): Rotations with noise + trace (density backend) =="
rm -f trace_dm.csv
"$BIN" --qasm circuits/qasm/rot1q.qasm --backend density \
  --dephase 0.05 --amp-damp 0.02 --depolarize 0.01 \
  --trace trace_dm.csv
echo "Wrote trace_dm.csv"

echo
echo "All demos done."
