import os
import time
import tqdm

import av

class PTSVideoIter:

    def __init__(self, video_f):
        self.video_f = video_f
        self.container = None
        self.dec_stream = None

    def __iter__(self):
        self.container = av.open(self.video_f)
        s = self.container.streams.video[0]
        self.dec_stream = self.container.decode(s)
        iter(self.dec_stream)
        return self

    def __next__(self):
        try:
            frame = next(self.dec_stream)
            return frame.pts # ms
        except StopIteration:
            raise

class Sampler:
    
    def __init__(self, pts_its, anchor_idx=0):
        self.anchor = pts_its.pop(anchor_idx)
        self.pts_its = pts_its
        self.lagging_pts = {}

    def __iter__(self):
        iter(self.anchor)
        for it in self.pts_its:
            self.lagging_pts[it.video_f] = []
            for pts in it:
                self.lagging_pts[it.video_f].append(pts)
        return self

    def __next__(self):
        try:
            a_pts = next(self.anchor)
            M = {self.anchor.video_f: a_pts}
            for it in self.pts_its:
                if self.lagging_pts[it.video_f][0] > a_pts:
                    return None
                got = False
                for t in range(1, self.lagging_pts[it.video_f]):
                    if self.lagging_pts[it.video_f][t] > a_pts:
                        M[it.video_f] = self.lagging_pts[it.video_f][t-1]
                        got = True
                        break
                if not got:
                    raise StopIteration
            return M
        except StopIteration:
            raise

def generate(mission_dir):
    # for each video file open and decode fully
    # create a json file
    """
    {
        0: {
            0: {"file1.mp4": t0, "file2.mp4": t0, {"file3.mp4": t0}},
            1: {"file1.mp4": t0, "file2.mp4": t0, {"file3.mp4": t0}},
            .
            .
        },
        1: {},
        .
        .
    }
    """ 

    manifest = {}

    for ep_idx, ep in enumerate(os.listdir(mission_dir)):

        ep = os.path.join(mission_dir, ep)
        data = os.path.join(ep, "data")

        left = PTSVideoIter(os.path.join(data, "left.perception_interface.camera.state.mp4"))
        middle = PTSVideoIter(os.path.join(data, "middle.perception_interface.camera.state.mp4"))
        right = PTSVideoIter(os.path.join(data, "right.perception_interface.camera.state.mp4"))

        manifest[ep_idx] = {}

        s = Sampler([left, middle, right], 0)
        for t, sample in tqdm.tqdm(enumerate(s)):
            if sample is not None:
                manifest[ep_idx][t] = s

    return manifest
