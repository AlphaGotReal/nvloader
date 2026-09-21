#! /usr/bin/env python3

import os
import sys

from IPython import embed
import argparse

from nvloader import manifest

def main(args):
    return manifest.generate(args.output)

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("-c", "--cont", action="store_true")
    parser.add_argument("--gvar", default="gvar")
    parser.add_argument("--output")
    args = parser.parse_args()

    _gvar = args.gvar
    globals()[_gvar] = main(args)

    if args.cont:
        embed()
