#!/usr/bin/env python3

import av
import os
import random
import argparse
from fractions import Fraction

def generate_episode_video(out_dir, episode_num, gop, fps=30):
    """Generate 3 MP4 videos for one episode (left, middle, right).
    
    Reads from /dev/video0 using PyAV demux/decode, encodes H264 to MP4
    with timebase 1/1000000, and records between 995-1004 frames per video.
    """
    data_dir = os.path.join(out_dir, f"episode{episode_num}", "data")
    os.makedirs(data_dir, exist_ok=True)

    # 3 camera positions: left, middle, right
    camera_names = ["left", "middle", "right"]

    for cam_name in camera_names:
        # Create filename: camera_name.perception_interface.camera.state.mp4
        filename = f"{cam_name}.perception_interface.camera.state.mp4"
        filepath = os.path.join(data_dir, filename)

        # Random frame count between 995 and 1004
        num_frames = 995 + random.randint(0, 10)

        # Use PyAV to read from /dev/video0 and encode to MP4
        in_container = av.open("/dev/video0")
        in_stream = in_container.streams.video[0]

        out_container = av.open(filepath, mode="w")
        out_stream = out_container.add_stream("libx264", rate=fps)
        out_stream.time_base = Fraction(1, 1000000)
        out_stream.width = in_stream.width
        out_stream.height = in_stream.height
        out_stream.pix_fmt = "yuv420p"
        out_stream.options = {"crf": "23"}

        frame_count = 0
        try:
            for packet in in_container.demux():
                if frame_count >= num_frames:
                    break
                for frame in in_stream.decode(packet):
                    if frame_count >= num_frames:
                        break
                    frame.pts = frame_count
                    frame_count += 1
                    print(frame_count)
                    for out_packet in out_stream.encode(frame):
                        out_container.mux(out_packet)
        except Exception as e:
            print(f"Error in episode {episode_num}, camera {cam_name}: {e}")

        # Final flush
        for out_packet in out_stream.encode(None):
            out_container.mux(out_packet)

        out_container.close()
        in_container.close()


def main(args):
    out_dir = args.output
    count = args.count
    gop = args.gop

    os.makedirs(out_dir, exist_ok=True)

    for t in range(count):
        print(f"Generating episode {t}...")
        generate_episode_video(out_dir, t, gop, fps=30)

    print(f"Done. Generated {count} episodes in {out_dir}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate episode videos with random frames")
    parser.add_argument("-o", "--output", default="episodes", help="Output directory")
    parser.add_argument("-c", "--count", type=int, default=5, help="Number of episodes")
    parser.add_argument("-g", "--gop", type=int, default=25, help="GOP size")
    args = parser.parse_args()
    main(args)
