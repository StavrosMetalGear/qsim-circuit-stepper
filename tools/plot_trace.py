import os
import sys
import csv
import math
import argparse
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D  # needed for 3D
import numpy as np


def read_trace_csv(path):
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)
    return rows


def get_series(rows, key):
    vals = []
    for r in rows:
        if key in r and r[key] != "":
            vals.append(float(r[key]))
    return vals


def save_line_plot(xs, ys, xlabel, ylabel, title, outpath):
    plt.figure()
    plt.plot(xs, ys, marker="o")
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.title(title)
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(outpath)
    plt.close()


def save_bloch_trajectory(xs, ys, zs, outpath):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection="3d")

    # Draw Bloch sphere wireframe
    u = np.linspace(0, 2 * np.pi, 60)
    v = np.linspace(0, np.pi, 30)
    X = np.outer(np.cos(u), np.sin(v))
    Y = np.outer(np.sin(u), np.sin(v))
    Z = np.outer(np.ones_like(u), np.cos(v))
    ax.plot_wireframe(X, Y, Z, rstride=3, cstride=3, linewidth=0.5, alpha=0.3)

    # Axes through the sphere
    ax.plot([-1, 1], [0, 0], [0, 0], alpha=0.4)
    ax.plot([0, 0], [-1, 1], [0, 0], alpha=0.4)
    ax.plot([0, 0], [0, 0], [-1, 1], alpha=0.4)

    # Trajectory
    ax.plot(xs, ys, zs, marker="o", linewidth=2)

    # Current/final Bloch vector as arrow
    if len(xs) > 0:
        ax.quiver(0, 0, 0, xs[-1], ys[-1], zs[-1], arrow_length_ratio=0.08)

    ax.set_title("Bloch trajectory")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.set_zlabel("z")

    ax.set_xlim([-1, 1])
    ax.set_ylim([-1, 1])
    ax.set_zlim([-1, 1])

    # Equal aspect ratio
    ax.set_box_aspect([1, 1, 1])

    plt.tight_layout()
    plt.savefig(outpath)
    plt.close()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csvfile")
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    os.makedirs(args.out, exist_ok=True)

    rows = read_trace_csv(args.csvfile)
    steps = list(range(len(rows)))

    bx = get_series(rows, "bloch_x")
    by = get_series(rows, "bloch_y")
    bz = get_series(rows, "bloch_z")
    purity = get_series(rows, "purity")
    coherence = get_series(rows, "coherence")
    entropy = get_series(rows, "ent_entropy")

    if bx:
        save_line_plot(steps[:len(bx)], bx, "step", "bloch_x", "Bloch X", os.path.join(args.out, "bloch_x.png"))
    if by:
        save_line_plot(steps[:len(by)], by, "step", "bloch_y", "Bloch Y", os.path.join(args.out, "bloch_y.png"))
    if bz:
        save_line_plot(steps[:len(bz)], bz, "step", "bloch_z", "Bloch Z", os.path.join(args.out, "bloch_z.png"))
    if purity:
        save_line_plot(steps[:len(purity)], purity, "step", "purity", "Purity", os.path.join(args.out, "purity.png"))
    if coherence:
        save_line_plot(steps[:len(coherence)], coherence, "step", "coherence", "Coherence", os.path.join(args.out, "coherence.png"))
    if entropy:
        save_line_plot(steps[:len(entropy)], entropy, "step", "ent_entropy", "Entanglement Entropy", os.path.join(args.out, "entropy.png"))

    if bx and by and bz:
        save_bloch_trajectory(bx, by, bz, os.path.join(args.out, "bloch_trajectory_3d.png"))


if __name__ == "__main__":
    main()