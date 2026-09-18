#! /usr/bin/env python3
import os
import sys
from torchcodec.decoders import VideoDecoder

if __name__ == "__main__":

    decoder = VideoDecoder(
        sys.argv[1],
        device="cuda",
    )

    L = len(decoder)
    print(L)

    frame = decoder[0]

    print(frame.type)
    print(frame.shape)
    print(frame.device)
