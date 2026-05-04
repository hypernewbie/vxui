"""
Run test_plot, render its layout dumps as PNG grids, optionally open them.

Usage:
    python tools/eyeball.py
    python tools/eyeball.py --build      # cmake build before running
    python tools/eyeball.py --show       # open the PNGs after writing
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path( __file__ ).resolve().parent.parent
TMP  = ROOT / "tmp"
EXE  = ROOT / "build" / "Debug" / "test_plot.exe"

SNAPSHOTS = [ "title_menu", "scrollable_menu", "menu_in_div", "root_absolute" ]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument( "--build", action="store_true" )
    ap.add_argument( "--show",  action="store_true" )
    args = ap.parse_args()

    if args.build:
        subprocess.check_call( [
            "cmake", "--build", str( ROOT / "build" ), "--config", "Debug",
            "--target", "test_plot"
        ] )

    TMP.mkdir( exist_ok=True )
    subprocess.check_call( [ str( EXE ) ], cwd=ROOT )

    plot = ROOT / "tools" / "plot_layout.py"

    snap_in  = [ str( TMP / f"{name}.layout" ) for name in SNAPSHOTS ]
    snap_out = TMP / "snapshots.png"
    subprocess.check_call( [
        sys.executable, str( plot ), *snap_in,
        "--grid", "2", "--out", str( snap_out )
    ] )

    spring_in = sorted( str( p ) for p in TMP.glob( "spring_*.layout" ) )
    spring_out = TMP / "spring_grid.png"
    if spring_in:
        subprocess.check_call( [
            sys.executable, str( plot ), *spring_in,
            "--grid", "6", "--out", str( spring_out )
        ] )

    print( f"\nsnapshots: {snap_out}" )
    if spring_in:
        print( f"spring:    {spring_out}" )

    if args.show:
        os.startfile( str( snap_out ) )
        if spring_in:
            os.startfile( str( spring_out ) )


if __name__ == "__main__":
    main()
