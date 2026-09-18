#! /usr/bin/env python3

import os
import sys

import random
import time
from tqdm import tqdm

import numpy as np
import torch
from torchcodec.decoders import VideoDecoder

if __name__ == "__main__":

    dec = VideoDecoder(
        sys.argv[1],
        device="cuda",
    )

    T = []

    for _ in tqdm(range(100)):
        idx = random.randint(0, len(dec) - 1)

        t0 = time.perf_counter()
        frame = dec[idx]
        # print(frame.device) # prints cuda
        torch.cuda.synchronize()
        dt = time.perf_counter() - t0

        # print(idx, dt)
        T.append(dt)

    T = np.array(T)
    print(T.mean())
    print(T.std())
    print(T.min())
    print(T.max())
