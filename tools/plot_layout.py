"""
Plot vxui .layout dumps as PNG via matplotlib.

Usage:
    python tools/plot_layout.py tmp/title.layout
    python tools/plot_layout.py tmp/spring_*.layout --grid 6
    python tools/plot_layout.py tmp/title.layout --out tmp/title.png --show

Reads each .layout file (produced by tests/dump_layout.h), renders rects as
matplotlib patches with HSL-from-id-hash colors, y-axis flipped to match
vxui screen-space coordinates.
"""

import argparse
import colorsys
import glob
import os
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import matplotlib.patches as mp


def parse_layout(path):
    w = h = dt = None
    rects = []
    with open(path, "r") as f:
        for ln, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            if line.startswith("#"):
                parts = line.split()
                if w is None and len(parts) >= 7 and parts[1] == "w" and parts[3] == "h":
                    w  = float(parts[2])
                    h  = float(parts[4])
                    dt = float(parts[6]) if len(parts) > 6 and parts[5] == "dt" else 0.0
                continue
            cols = line.split(None, 5)
            assert len(cols) >= 5, f"{path}:{ln}: expected at least 5 cols, got {cols}"
            rid   = cols[0]
            x     = float(cols[1])
            y     = float(cols[2])
            rw    = float(cols[3])
            rh    = float(cols[4])
            label = cols[5] if len(cols) > 5 else "-"
            rects.append((rid, x, y, rw, rh, label))

    assert w is not None and h is not None, f"{path}: missing # w H header"
    return w, h, dt or 0.0, rects


def color_for_id(rid):
    h = int(rid, 16) & 0xFFFFFFFF
    hue = ((h * 2654435761) & 0xFFFFFFFF) / 0xFFFFFFFF
    r, g, b = colorsys.hls_to_rgb(hue, 0.65, 0.55)
    return (r, g, b)


def short_id(rid):
    return rid[:6] if rid.startswith("0x") else rid


def render_one(ax, w, h, rects, title, min_w, min_h, full):
    if full or not rects:
        x0, y0, x1, y1 = 0, 0, w, h
    else:
        xs = [r[1]                           for r in rects]
        ys = [r[2]                           for r in rects]
        xe = [r[1] + (r[3] if r[3] > 0 else min_w) for r in rects]
        ye = [r[2] + (r[4] if r[4] > 0 else min_h) for r in rects]
        cx0, cy0 = min(xs), min(ys)
        cx1, cy1 = max(xe), max(ye)
        mx = max( ( cx1 - cx0 ) * 0.15, 16 )
        my = max( ( cy1 - cy0 ) * 0.15, 16 )
        x0, y0, x1, y1 = cx0 - mx, cy0 - my, cx1 + mx, cy1 + my

    ax.set_xlim(x0, x1)
    ax.set_ylim(y0, y1)
    ax.invert_yaxis()
    ax.set_aspect("equal")
    ax.set_title(title, fontsize=9)
    ax.tick_params(labelsize=7)

    for rid, x, y, rw, rh, label in rects:
        viz_w = rw if rw > 0 else min_w
        viz_h = rh if rh > 0 else min_h
        is_focus = label.startswith("<") and label.endswith(">")
        if is_focus:
            rect = mp.Rectangle((x, y), viz_w, viz_h,
                                facecolor="none", edgecolor="red",
                                linewidth=2.0, linestyle="--")
            ax.add_patch(rect)
        else:
            color = color_for_id(rid)
            rect = mp.Rectangle((x, y), viz_w, viz_h,
                                facecolor=color, edgecolor="black",
                                linewidth=0.8, alpha=0.55)
            ax.add_patch(rect)
            text = label if label != "-" else short_id(rid)
            ax.text(x + viz_w / 2, y + viz_h / 2, text,
                    ha="center", va="center", fontsize=6, color="black")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("inputs", nargs="+", help="layout file(s) or glob(s)")
    ap.add_argument("--out", default=None, help="output PNG path")
    ap.add_argument("--grid", type=int, default=0,
                    help="N columns; arrange multiple inputs in a grid")
    ap.add_argument("--show", action="store_true", help="open PNG after writing")
    ap.add_argument("--min-w", type=float, default=200.0,
                    help="substitute 0-width rects with this for visibility")
    ap.add_argument("--min-h", type=float, default=8.0,
                    help="substitute 0-height rects with this for visibility")
    ap.add_argument("--full", action="store_true",
                    help="show full viewport instead of zooming to content")
    args = ap.parse_args()

    paths = []
    for inp in args.inputs:
        matches = glob.glob(inp)
        if matches:
            paths.extend(sorted(matches))
        elif os.path.exists(inp):
            paths.append(inp)
        else:
            print(f"no match: {inp}", file=sys.stderr)
    assert paths, "no input files matched"

    if len(paths) == 1 or args.grid == 0:
        for p in paths:
            w, h, dt, rects = parse_layout(p)
            fig, ax = plt.subplots(figsize=(8, 8 * h / w))
            render_one(ax, w, h, rects,
                       f"{Path(p).name}  ({len(rects)} rects, dt={dt:.4f})",
                       args.min_w, args.min_h, args.full)
            out = args.out or str(Path(p).with_suffix(".png"))
            fig.tight_layout()
            fig.savefig(out, dpi=120)
            plt.close(fig)
            print(f"wrote {out}")
            if args.show: os.startfile(out)
        return

    cols = args.grid
    rows = (len(paths) + cols - 1) // cols
    w0, h0, _, _ = parse_layout(paths[0])
    fig, axes = plt.subplots(rows, cols,
                             figsize=(cols * 4, rows * 4 * h0 / w0))
    if rows == 1 and cols == 1: axes = [[axes]]
    elif rows == 1: axes = [axes]
    elif cols == 1: axes = [[a] for a in axes]

    for i, p in enumerate(paths):
        r, c = divmod(i, cols)
        w, h, dt, rects = parse_layout(p)
        render_one(axes[r][c], w, h, rects,
                   f"{Path(p).name}  ({len(rects)} rects)",
                   args.min_w, args.min_h, args.full)
    for i in range(len(paths), rows * cols):
        r, c = divmod(i, cols)
        axes[r][c].axis("off")

    out = args.out or os.path.join(os.path.dirname(paths[0]) or ".", "grid.png")
    fig.tight_layout()
    fig.savefig(out, dpi=120)
    plt.close(fig)
    print(f"wrote {out}")
    if args.show: os.startfile(out)


if __name__ == "__main__":
    main()
