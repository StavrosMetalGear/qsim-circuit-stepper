#!/usr/bin/env python3
import argparse
import csv
from pathlib import Path

def read_csv(path: Path):
    rows = []
    with path.open("r", newline="") as f:
        r = csv.DictReader(f)
        for row in r:
            rows.append(row)
    return rows

def to_float(x, default=None):
    if x is None: return default
    s = str(x).strip()
    if s == "": return default
    try:
        return float(s)
    except ValueError:
        return default

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv", type=Path, help="trace csv file (e.g. trace_dm.csv)")
    ap.add_argument("--out", type=Path, default=Path("plots"), help="output folder")
    ap.add_argument("--no-3d", action="store_true", help="skip 3D Bloch trajectory plot")
    args = ap.parse_args()

    rows = read_csv(args.csv)
    if not rows:
        raise SystemExit("No rows found in CSV.")

    steps = [int(r["step"]) for r in rows]
    x = [to_float(r.get("x"), 0.0) for r in rows]
    y = [to_float(r.get("y"), 0.0) for r in rows]
    z = [to_float(r.get("z"), 0.0) for r in rows]
    purity = [to_float(r.get("purity"), 0.0) for r in rows]
    coherence = [to_float(r.get("coherence"), 0.0) for r in rows]
    entropy = [to_float(r.get("entropy_bits"), 0.0) for r in rows]

    args.out.mkdir(parents=True, exist_ok=True)

    import matplotlib.pyplot as plt
    from mpl_toolkits.mplot3d import Axes3D  # noqa: F401

    def save_2d(xs, ys, ylabel, fname):
        plt.figure()
        plt.plot(xs, ys)
        plt.xlabel("step")
        plt.ylabel(ylabel)
        plt.tight_layout()
        plt.savefig(args.out / fname, dpi=160)
        plt.close()

    save_2d(steps, x, "Bloch x", "bloch_x.png")
    save_2d(steps, y, "Bloch y", "bloch_y.png")
    save_2d(steps, z, "Bloch z", "bloch_z.png")
    save_2d(steps, purity, "purity", "purity.png")
    save_2d(steps, coherence, "coherence", "coherence.png")
    save_2d(steps, entropy, "entropy (bits)", "entropy.png")

    if not args.no_3d:
        fig = plt.figure()
        ax = fig.add_subplot(111, projection="3d")
        ax.plot(x, y, z)
        ax.set_xlabel("x")
        ax.set_ylabel("y")
        ax.set_zlabel("z")
        ax.set_title("Bloch trajectory")
        plt.tight_layout()
        plt.savefig(args.out / "bloch_trajectory_3d.png", dpi=160)
        plt.close()

    print(f"Wrote plots to: {args.out}")

if __name__ == "__main__":
    main()
