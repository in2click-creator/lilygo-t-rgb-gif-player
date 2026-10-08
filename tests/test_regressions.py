"""Exercise malformed LZW sizes and the real decoder/timing boundary."""
import os
from pathlib import Path
import subprocess
import tempfile

from PIL import Image

ROOT = Path(__file__).resolve().parent
RUNNER = Path(os.environ.get("DECODER_TEST", str(ROOT / "decoder_test"))).resolve()
DEVICE = ROOT / "fixtures" / "device" / "gif"


def run(path, prefix):
    return subprocess.run(
        [str(RUNNER), str(path), str(prefix)],
        capture_output=True, text=True, timeout=5,
    )


def steps(result):
    assert result.returncode == 0, result.stdout + result.stderr
    return [tuple(map(int, line.split()[1:])) for line in result.stdout.splitlines()
            if line.startswith("STEP ")]


def first_code_size_offset(data):
    """Walk GIF blocks; don't mistake a palette/payload byte for a separator."""
    offset = 13
    if data[10] & 128:
        offset += 3 * (1 << ((data[10] & 7) + 1))
    while data[offset] == 0x21:
        offset += 2
        while data[offset]:
            offset += 1 + data[offset]
        offset += 1
    assert data[offset] == 0x2C
    packed = data[offset + 9]
    offset += 10
    if packed & 128:
        offset += 3 * (1 << ((packed & 7) + 1))
    return offset


def main():
    DEVICE.mkdir(parents=True, exist_ok=True)
    normal = DEVICE / "001-normal.gif"
    commented = DEVICE / "002-trailing-comment.gif"
    malformed = DEVICE / "003-invalid-lzw.gif"
    frames = [Image.new("RGB", (96, 96), colour)
              for colour in ("red", "lime", "blue", "yellow")]
    frames[0].save(normal, save_all=True, append_images=frames[1:],
                   duration=[50, 100, 150, 200], loop=0, disposal=1)
    data = normal.read_bytes()
    assert data[-1] == 0x3B
    # Valid metadata after the last image forces an EOF call with no new frame.
    commented.write_bytes(data[:-1] + b"!\xfe\x14" + b"A" * 20 + b"\x00;")
    code_offset = first_code_size_offset(data)
    broken = bytearray(data)
    broken[code_offset] = 12  # Original ASan reproduction; keep for device checks.
    malformed.write_bytes(broken)

    with tempfile.TemporaryDirectory() as directory:
        temp = Path(directory)
        ordinary = steps(run(normal, temp / "normal"))
        trailing = steps(run(commented, temp / "comment"))
        for result in (ordinary, trailing):
            rendered = [s for s in result if s[1]]
            assert len(rendered) == 4, result
            assert [s[3] for s in rendered] == [50, 100, 150, 200], result
            assert sum(s[3] for s in result) == 500, result
        assert trailing[-1] == (0, 0, 0, 0), trailing
        for frame in range(4):
            assert (temp / f"normal-{frame}.raw").read_bytes() == (
                temp / f"comment-{frame}.raw").read_bytes()
        print("PASS trailing metadata: identical frames and 500 ms per loop")

        # Every byte outside the legal 2..8 range must fail cleanly, not crash.
        for code_size in (0, 1, *range(9, 256)):
            broken[code_offset] = code_size
            candidate = temp / "bad.gif"
            candidate.write_bytes(broken)
            result = run(candidate, temp / "bad")
            assert result.returncode == 5 and "DECODE ERROR" in result.stdout, (
                code_size, result.returncode, result.stdout, result.stderr)
        print("PASS all 249 invalid LZW code-size bytes are rejected")
    print(f"Device fixtures: {DEVICE}")


if __name__ == "__main__":
    main()
