#! /usr/bin/env python3

import os
import sys

from IPython import embed
import argparse

def main(args):
    return 

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("-c", "--cont", action="store_true")
    parser.add_argument("--gvar", default="gvar")
    args = parser.parse_args()

    _gvar = args.gvar
    globals()[_gvar] = main(args)

    if args.cont:
        embed()
