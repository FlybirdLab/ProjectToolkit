#!/usr/bin/env python3
# scripts/fix.py

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path


ZSTD_MAGIC = b"\x28\xb5\x2f\xfd"

SKIP_DIRS = {
    ".git",
    ".svn",
    ".hg",
    ".idea",
    ".vscode",
    "build",
    "cmake-build-debug",
    "cmake-build-release",
    "node_modules",
    ".gradle",
    ".dart_tool",
    "__pycache__",
}

SKIP_EXTS = {
    ".exe", ".dll", ".so", ".dylib",
    ".lib", ".a", ".o", ".obj",
    ".class", ".jar",
    ".png", ".jpg", ".jpeg", ".webp", ".gif", ".ico",
    ".mp3", ".wav", ".ogg", ".flac",
    ".mp4", ".mkv", ".avi",
    ".zip", ".7z", ".rar", ".tar", ".gz", ".xz",
}


def is_zstd_file(path: Path) -> bool:
    try:
        with path.open("rb") as f:
            return f.read(4) == ZSTD_MAGIC
    except OSError:
        return False


def looks_like_text(data: bytes) -> bool:
    if not data:
        return True

    sample = data[:8192]

    if b"\x00" in sample:
        return False

    try:
        data.decode("utf-8")
        return True
    except UnicodeDecodeError:
        pass

    printable = 0
    for b in sample:
        if b in (9, 10, 13) or 32 <= b <= 126 or b >= 128:
            printable += 1

    return printable / max(1, len(sample)) >= 0.90


def decompress_by_zstd_cli(path: Path) -> tuple[bytes | None, str]:
    zstd = shutil.which("zstd")
    if not zstd:
        return None, "zstd.exe not found in PATH"

    try:
        p = subprocess.run(
            [zstd, "-d", "-c", str(path)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            shell=False,
        )
    except Exception as e:
        return None, f"zstd cli error: {e}"

    # 关键：这类坏文件可能 returncode != 0，但 stdout 已经是正确源码。
    if p.stdout:
        return p.stdout, ""

    err = p.stderr.decode("utf-8", errors="replace").strip()
    return None, f"zstd cli failed: {err}"


def decompress_by_python_zstandard(path: Path) -> tuple[bytes | None, str]:
    try:
        import zstandard as zstd
    except ImportError:
        return None, "python package zstandard not installed"

    try:
        dctx = zstd.ZstdDecompressor()

        # 用 stream_reader，比一次性 decompress 更能处理 unknown size。
        with path.open("rb") as f:
            with dctx.stream_reader(f, read_across_frames=False) as reader:
                chunks: list[bytes] = []
                while True:
                    chunk = reader.read(1024 * 1024)
                    if not chunk:
                        break
                    chunks.append(chunk)

        data = b"".join(chunks)
        if data:
            return data, ""

        return None, "python zstandard produced empty output"

    except Exception as e:
        return None, f"python zstandard failed: {e}"


def decompress_file(path: Path) -> tuple[bytes | None, str]:
    # 先用 zstd.exe，因为你这种 malformed/trailing garbage 的文件，
    # zstd.exe 有时能吐出正确 stdout。
    data, err1 = decompress_by_zstd_cli(path)
    if data:
        return data, ""

    data, err2 = decompress_by_python_zstandard(path)
    if data:
        return data, ""

    return None, f"{err1}; {err2}"


def make_backup_path(path: Path) -> Path:
    p = path.with_name(path.name + ".zstbak")
    if not p.exists():
        return p

    for i in range(1, 10000):
        p = path.with_name(f"{path.name}.zstbak.{i}")
        if not p.exists():
            return p

    raise RuntimeError(f"too many backups for {path}")


def restore_one(path: Path, write: bool, no_backup: bool, allow_binary: bool) -> str:
    if not is_zstd_file(path):
        return "skip"

    data, err = decompress_file(path)
    if not data:
        return f"failed: cannot decompress: {err}"

    if not allow_binary and not looks_like_text(data):
        return "skip: decompressed data is not text"

    if not write:
        return f"would restore: {len(data)} bytes"

    if not no_backup:
        backup = make_backup_path(path)
        shutil.copy2(path, backup)

    tmp = path.with_name(path.name + ".tmp_restore")

    try:
        tmp.write_bytes(data)

        try:
            shutil.copystat(path, tmp, follow_symlinks=False)
        except OSError:
            pass

        os.replace(tmp, path)
        return f"restored: {len(data)} bytes"

    finally:
        if tmp.exists():
            try:
                tmp.unlink()
            except OSError:
                pass


def iter_files(root: Path, include_binary_exts: bool):
    if root.is_file():
        yield root
        return

    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]

        base = Path(dirpath)

        for name in filenames:
            p = base / name

            if not include_binary_exts and p.suffix.lower() in SKIP_EXTS:
                continue

            # 不处理备份文件，避免二次处理
            if ".zstbak" in p.name:
                continue

            yield p


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("paths", nargs="+")
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--no-backup", action="store_true")
    parser.add_argument("--allow-binary", action="store_true")
    parser.add_argument("--include-binary-exts", action="store_true")
    args = parser.parse_args()

    total = 0
    matched = 0
    restored = 0
    failed = 0

    for raw in args.paths:
        root = Path(raw)

        if not root.exists():
            print(f"[missing] {root}")
            failed += 1
            continue

        for path in iter_files(root, args.include_binary_exts):
            total += 1

            if not is_zstd_file(path):
                continue

            matched += 1

            result = restore_one(
                path=path,
                write=args.write,
                no_backup=args.no_backup,
                allow_binary=args.allow_binary,
            )

            print(f"[{result}] {path}")

            if result.startswith("restored"):
                restored += 1
            elif result.startswith("failed"):
                failed += 1

    print()
    print(f"scanned:  {total}")
    print(f"matched:  {matched}")
    print(f"restored: {restored}")
    print(f"failed:   {failed}")

    if not args.write:
        print()
        print("dry-run only. add --write to actually restore files.")

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())