#!/usr/bin/env python3

import cv2
import av
import queue
import threading
import os
import random
import argparse
from fractions import Fraction


def capture_frames(cap, frame_queue, num_frames):
    """Capture frames from OpenCV camera into queue."""
    for _ in range(num_frames):
        ret, frame_bgr = cap.read()
        if not ret:
            break
        frame_queue.put(frame_bgr)
    frame_queue.put(None)  # Signal end of frames


def encode_video(frame_queue, output_path, width, height, fps, num_frames):
    """Encode frames from queue to H264 MP4 using PyAV."""
    container = av.open(output_path, mode="w")
    stream = container.add_stream("libx264", rate=fps)
    stream.time_base = Fraction(1, 1000000)
    stream.width = width
    stream.height = height
    stream.pix_fmt = "yuv420p"
    stream.options = {"crf": "23"}

    frame_count = 0
    while frame_count < num_frames:
        try:
            frame_bgr = frame_queue.get(timeout=5)
        except queue.Empty:
            break

        if frame_bgr is None:
            break

        frame_rgb = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB)
        av_frame = av.VideoFrame.from_ndarray(
            frame_rgb.reshape(height, width, 3), format="rgb24"
        )
        av_frame.pts = frame_count
        frame_count += 1

        for packet in stream.encode(av_frame):
            container.mux(packet)

    for packet in stream.encode(None):
        container.mux(packet)

    container.close()


def generate_episode_video(out_dir, episode_num, gop, fps=30):
    """Generate 3 MP4 videos for one episode (left, middle, right)."""
    data_dir = os.path.join(out_dir, f"episode{episode_num}", "data")
    os.makedirs(data_dir, exist_ok=True)

    camera_names = ["left", "middle", "right"]

    for cam_name in camera_names:
        filename = f"{cam_name}.perception_interface.camera.state.mp4"
        filepath = os.path.join(data_dir, filename)
        num_frames = 995 + random.randint(0, 9)

        print(f"  {cam_name}: {num_frames} frames -> {filepath}")

        cap = cv2.VideoCapture(0)
        if not cap.isOpened():
            print(f"  Error: Cannot open camera")
            continue

        width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

        frame_queue = queue.Queue(maxsize=300)

        capture_thread = threading.Thread(
            target=capture_frames, args=(cap, frame_queue, num_frames)
        )
        capture_thread.start()

        encode_video(frame_queue, filepath, width, height, fps, num_frames)

        capture_thread.join()
        cap.release()


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