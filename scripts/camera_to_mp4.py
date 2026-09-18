#!/usr/bin/env python3

import av
import signal
import threading
import argparse
from fractions import Fraction

def main(args):
    in_container = av.open(args.device)
    in_stream = in_container.streams.video[0]

    width = in_stream.width
    height = in_stream.height
    fps_raw = in_stream.average_rate
    print(f"Camera: {width}x{height} @ {fps_raw} from {args.device}")

    out_container = av.open(args.output, mode="w")
    out_stream = out_container.add_stream("libx264", rate=fps_raw)
    out_stream.time_base = Fraction(1, 1000000)
    out_stream.width = width
    out_stream.height = height
    out_stream.pix_fmt = "yuv420p"
    out_stream.options = {"crf": str(args.crf)}

    print(f"Writing H264 to {args.output} ... Press Ctrl+C to stop.")

    stop_event = threading.Event()

    def handler(sig, frame):
        stop_event.set()

    signal.signal(signal.SIGINT, handler)

    frame_count = 0
    try:
        for packet in in_container.demux():
            if stop_event.is_set():
                break
            for frame in in_stream.decode(packet):
                frame.pts = frame_count
                frame_count += 1
                for out_packet in out_stream.encode(frame):
                    out_container.mux(out_packet)
    except KeyboardInterrupt:
        pass

    for packet in out_stream.encode(None):
        out_container.mux(packet)

    out_container.close()
    in_container.close()
    print("Done.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Capture camera to H264 MP4 using pyav")
    parser.add_argument("-d", "--device", default="/dev/video0", help="Video device path")
    parser.add_argument("-o", "--output", default="output.mp4", help="Output MP4 filename")
    parser.add_argument("-c", "--crf", type=int, default=23, help="H264 CRF (lower = better quality)")
    args = parser.parse_args()
    main(args)
