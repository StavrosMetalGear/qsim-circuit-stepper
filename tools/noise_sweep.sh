#!/usr/bin/env bash
set -euo pipefail

BIN="${1:-./build/qsim_stepper}"
QASM="${2:-circuits/qasm/bell.qasm}"

# Sweep mode: currently sweeps dephasing only (easy + meaningful)
# You can extend later to depolarize or amp-damp sweeps.

OUT="${3:-noise_sweep_dephase.csv}"
SHOTS="${SHOTS:-1000}"
SEED="${SEED:-7}"

# Fixed noise parameters (except the swept one)
DEPOL="${DEPOL:-0.00}"
AMPD="${AMPD:-0.00}"

# Range for dephase sweep (inclusive):
# e.g. 0.00, 0.02, ... 0.20
START="${START:-0.00}"
STOP="${STOP:-0.20}"
STEP="${STEP:-0.02}"

if [[ ! -x "$BIN" ]]; then
  echo "ERROR: binary not executable: $BIN"
  echo "Build first: cmake -S . -B build && cmake --build build -j"
  exit 1
fi

if [[ ! -f "$QASM" ]]; then
  echo "ERROR: QASM file not found: $QASM"
  exit 1
fi

echo "Writing: $OUT"
echo "Using: BIN=$BIN QASM=$QASM SHOTS=$SHOTS SEED=$SEED DEPOL=$DEPOL AMPD=$AMPD"
echo "Sweep dephase from $START to $STOP step $STEP"

# CSV header
echo "dephase,depolarize,amp_damp,shots,p00,p01,p10,p11,purity_s1,coherence_s1,ent_entropy_s1" > "$OUT"

# helper to format floats consistently
fmt() { printf "%.2f" "$1"; }

# floating loop without python: use awk
awk -v s="$START" -v e="$STOP" -v step="$STEP" 'BEGIN{
  for (x=s; x<=e+1e-12; x+=step) printf "%.10f\n", x;
}' | while read -r DEPHASE; do
  # ---- 1) Shots run (histogram) ----
  SHOT_OUT="$("$BIN" --qasm "$QASM" --backend density \
    --shots "$SHOTS" --seed "$SEED" \
    --depolarize "$DEPOL" --dephase "$DEPHASE" --amp-damp "$AMPD" \
    2>/dev/null || true
  )"

  # parse counts like:
  # 00 : 245
  # 11 : 255
  c00=$(echo "$SHOT_OUT" | awk '/^00[[:space:]]*:/ {print $3}' | tail -n1); c00=${c00:-0}
  c01=$(echo "$SHOT_OUT" | awk '/^01[[:space:]]*:/ {print $3}' | tail -n1); c01=${c01:-0}
  c10=$(echo "$SHOT_OUT" | awk '/^10[[:space:]]*:/ {print $3}' | tail -n1); c10=${c10:-0}
  c11=$(echo "$SHOT_OUT" | awk '/^11[[:space:]]*:/ {print $3}' | tail -n1); c11=${c11:-0}

  # ---- 2) Step mode run (metrics at step 1) ----
  # We run without shots; your program prints metrics per step.
  STEP_OUT="$("$BIN" --qasm "$QASM" --backend density \
    --seed "$SEED" \
    --depolarize "$DEPOL" --dephase "$DEPHASE" --amp-damp "$AMPD" \
    2>/dev/null || true
  )"

  # Find the line for step 1 that contains purity/coherence/ent_entropy
  # Example:
  # step 1 | q0 purity=0.500000 coherence=0.000000 ... ent_entropy=1.000000 bits
  MET_LINE=$(echo "$STEP_OUT" | awk '/^step 1 .*purity=.*coherence=.*ent_entropy=/ {print; exit}')

  purity=$(echo "$MET_LINE" | sed -n 's/.*purity=\([0-9.]\+\).*/\1/p'); purity=${purity:-}
  coh=$(echo "$MET_LINE" | sed -n 's/.*coherence=\([0-9.]\+\).*/\1/p'); coh=${coh:-}
  ent=$(echo "$MET_LINE" | sed -n 's/.*ent_entropy=\([0-9.]\+\).*/\1/p'); ent=${ent:-}

  # Convert counts to probabilities
  p00=$(awk -v c="$c00" -v n="$SHOTS" 'BEGIN{printf "%.6f", (n>0?c/n:0)}')
  p01=$(awk -v c="$c01" -v n="$SHOTS" 'BEGIN{printf "%.6f", (n>0?c/n:0)}')
  p10=$(awk -v c="$c10" -v n="$SHOTS" 'BEGIN{printf "%.6f", (n>0?c/n:0)}')
  p11=$(awk -v c="$c11" -v n="$SHOTS" 'BEGIN{printf "%.6f", (n>0?c/n:0)}')

  # Write row
  echo "$(awk -v x="$DEPHASE" 'BEGIN{printf "%.6f", x}'),$(awk -v x="$DEPOL" 'BEGIN{printf "%.6f", x}'),$(awk -v x="$AMPD" 'BEGIN{printf "%.6f", x}'),$SHOTS,$p00,$p01,$p10,$p11,$purity,$coh,$ent" >> "$OUT"

  echo "dephase=$(awk -v x="$DEPHASE" 'BEGIN{printf "%.3f", x}')  p00=$p00 p11=$p11  ent1=${ent:-NA}"
done

echo "Done. Output: $OUT"
