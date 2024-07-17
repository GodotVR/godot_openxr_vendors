#!/usr/bin/env python

import argparse
import numpy
import sys
from pathlib import Path

import pyvrs
from PIL import Image
from pixelmatch.contrib.PIL import pixelmatch
from matplotlib import pyplot as plt

class Stream:
    def __init__(self, stream_id, filtered_reader):
        self.stream_id = stream_id
        self.filtered_reader = filtered_reader

class FrameGroupMetadata:
    def __init__(self, frame_count, timestamp):
        self.frame_count = frame_count
        self.timestamp = timestamp

class Frame:
    def __init__(self, record):
        pixels = numpy.array(record.image_blocks[0])
        self.image = Image.fromarray(numpy.uint8(pixels)).convert('RGBA')
        self.timestamp = record.timestamp

def getStreams(reader, prefix):
    ret = []

    for stream_id in reader.stream_ids:
        if not stream_id.startswith(prefix):
            continue

        filtered_reader = reader.filtered_by_fields(stream_ids = stream_id, record_types = 'data')

        ret.append(Stream(stream_id, filtered_reader))

    return ret

def getFrameGroups(reader, idx=0):
    streams = getStreams(reader, '400')

    ret = []

    for record in streams[idx].filtered_reader:
        ret.append(FrameGroupMetadata(record.metadata_blocks[0]['iValue'], record.timestamp))

    return ret

def getFrame(stream, idx):
    if idx < 0 or idx >= len(stream.filtered_reader):
        return None
    return Frame(stream.filtered_reader[idx])

def displayFrames(recording, replay):
    plt.figure(figsize=(20, 10))

    plt.subplot(1, 2, 1)
    plt.imshow(numpy.asarray(recording.image), vmin=0, vmax=255, cmap='gray')
    plt.title('Recording')
    plt.axis('off')

    plt.subplot(1, 2, 2)
    plt.imshow(numpy.asarray(replay.image), vmin=0, vmax=255, cmap='gray')
    plt.title('Replay')
    plt.axis('off')

    plt.show()

def main():
    parser = argparse.ArgumentParser(description='Compare screenshots saved to VRS files.')
    parser.add_argument('recording', type=str, help="VRS file with original recording")
    parser.add_argument('replay', type=str, help="VRS file with replay to check")
    parser.add_argument('--max-pixel-diff', type=int, required=False, default=100, help="The maximum number of pixel differences to consider screenshots to be the same")
    parser.add_argument('--output-path', type=str, required=False, default=None, help="The output directory to write image diffs")
    parser.add_argument('--threshold', type=float, required=False, default=0.1, help="The threshold over which the pixels are considered to be different")
    parser.add_argument('--sample-location', type=float, required=False, default=0.5, help="Where in the group of frames to test: 0=beginning, 0.5=middle, 1=end")
    parser.add_argument('--show-frames', action='store_true', required=False, help="Display the matching frames side-by-side")
    parser.add_argument('--clamp-group-count', action='store_true', required=False, help="Perform a comparison even when the group count doesn't match, clamping to the shortest count.")
    parser.add_argument('--verbose', action='store_true', required=False, help="Print verbose messages")
    args = parser.parse_args()

    recording_path = Path(args.recording)
    recording_reader = pyvrs.SyncVRSReader(recording_path)

    replay_path = Path(args.replay)
    replay_reader = pyvrs.SyncVRSReader(replay_path)

    recording_groups = getFrameGroups(recording_reader)
    replay_groups = getFrameGroups(replay_reader)

    group_count = len(recording_groups)

    if group_count != len(replay_groups):
        if args.clamp_group_count:
            group_count = min(group_count, len(replay_groups))
            print(f'Group counts do not match: recording has {len(recording_groups)} groups and replay has {len(replay_groups)} - clamping to {group_count}')
        else:
            print(f'Cannot compare VRS files: recording has {len(recording_groups)} groups and replay has {len(replay_groups)}')
            sys.exit(1)

    if args.verbose:
        print(f'Comparing {group_count} groups of screenshots')

    recording_frame_stream = getStreams(recording_reader, "8003")[0]
    replay_frame_stream = getStreams(replay_reader, "8003")[0]

    matched = True

    frame_offset = 0
    for group_index in range(group_count):
        assert recording_groups[group_index].frame_count == replay_groups[group_index].frame_count

        frame_count = recording_groups[group_index].frame_count
        sample_index = frame_offset + int(args.sample_location * frame_count)

        if args.verbose:
            print(f'Group {group_index} with {frame_count} frames - sampling index {sample_index}')

        recording_frame = getFrame(recording_frame_stream, sample_index)
        replay_frame = getFrame(replay_frame_stream, sample_index)

        if args.show_frames:
            displayFrames(recording_frame, replay_frame)

        image_diff = None
        if args.output_path:
            image_diff = Image.new("RGBA", recording_frame.image.size)

        pixel_diff = pixelmatch(recording_frame.image, replay_frame.image, output=image_diff, threshold=args.threshold, includeAA=True)
        if pixel_diff > args.max_pixel_diff:
            matched = False
            print(f'Index {sample_index:03}: Pixel difference of {pixel_diff} exceeds maximum of {args.max_pixel_diff} - this is a mismatch!')

            if args.output_path:
                output_path = Path(args.output_path)
                recording_frame.image.save(output_path / f'{sample_index:03}-recording.png')
                replay_frame.image.save(output_path / f'{sample_index:03}-replay.png')
                image_diff.save(output_path / f'{sample_index:03}-diff.png')

        frame_offset += frame_count

    if not matched:
        sys.exit(1)


if __name__ == '__main__': main()
