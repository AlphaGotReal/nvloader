#! /usr/bin/env python3

import os
import sys
import json

from IPython import embed
import argparse

from nvloader import manifest

def main(args):
    M = manifest.generate(args.mission)
    with open(os.path.join(args.mission, "manifest.json"), "w") as f:
        json.dump(M, f, indent=2)
    return M

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("-c", "--cont", action="store_true")
    parser.add_argument("--gvar", default="gvar")
    parser.add_argument("--mission")
    args = parser.parse_args()

    _gvar = args.gvar
    globals()[_gvar] = main(args)

    if args.cont:
        embed()
