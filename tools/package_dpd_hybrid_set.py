#!/usr/bin/env python3
"""Package native/sparse DPD deployment files as a PDFxTMD-DPDH1 set."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import shutil
import struct


NATIVE_MAGIC = b"DPDNAT1\0"
HYBRID_MAGIC = b"DPD-HYBRID-B1\0\0\0"
FORMAT_NAME = "PDFxTMD-DPDH1"


def _checkpoint_from_native(path: Path) -> bytes:
    with path.open("rb") as stream:
        header = stream.read(132)
    if len(header) < 132 or header[:8] != NATIVE_MAGIC:
        raise SystemExit(f"not a DPDNAT1 neural model: {path}")
    version = struct.unpack_from("<I", header, 8)[0]
    if version != 1:
        raise SystemExit(f"unsupported DPDNAT1 version {version}: {path}")
    _, _, _, _, flags, reserved = struct.unpack_from("<6I", header, 12)
    if flags != 1 or reserved != 0:
        raise SystemExit(f"not an asinh v1 DPDNAT1 model: {path}")
    return header[100:132]


def _checkpoint_from_hybrid(path: Path) -> bytes:
    with path.open("rb") as stream:
        header = stream.read(116)
    if len(header) < 116 or header[:16] != HYBRID_MAGIC:
        raise SystemExit(f"not a DPD-HYBRID-B1 artifact: {path}")
    version = struct.unpack_from("<I", header, 16)[0]
    if version != 1:
        raise SystemExit(f"unsupported DPD-HYBRID-B1 version {version}: {path}")
    return header[84:116]


def _replace_yaml_scalar(text: str, key: str, value: str) -> str:
    pattern = re.compile(rf"(?m)^{re.escape(key)}\s*:.*$")
    replacement = f"{key}: {value}"
    if pattern.search(text):
        return pattern.sub(replacement, text, count=1)
    if text and not text.endswith("\n"):
        text += "\n"
    return text + replacement + "\n"


def _atomic_copy(source: Path, target: Path) -> None:
    temporary = target.with_suffix(target.suffix + ".tmp")
    shutil.copyfile(source, temporary)
    os.replace(temporary, target)


def _atomic_text(text: str, target: Path) -> None:
    temporary = target.with_suffix(target.suffix + ".tmp")
    temporary.write_text(text)
    os.replace(temporary, target)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--native", type=Path, required=True, help="DPDNAT1 neural model")
    parser.add_argument(
        "--hybrid", type=Path, required=True, help="DPD-HYBRID-B1 sparse artifact"
    )
    parser.add_argument(
        "--source-info", type=Path, required=True, help="metadata template for the source DPD set"
    )
    parser.add_argument("--output-root", type=Path, required=True)
    parser.add_argument("--set-name", required=True)
    parser.add_argument("--member", type=int, default=0)
    parser.add_argument("--num-members", type=int, default=1)
    parser.add_argument(
        "--cache-capacity",
        type=int,
        help="Optional HybridCacheCapacity in neural-grid nodes; zero disables caching",
    )
    parser.add_argument(
        "--threads",
        type=int,
        help="Optional positive HybridThreads preference for hybrid inference",
    )
    parser.add_argument("--force", action="store_true")
    args = parser.parse_args()

    for label, path in (
        ("native model", args.native),
        ("hybrid artifact", args.hybrid),
        ("source info", args.source_info),
    ):
        if not path.is_file():
            raise SystemExit(f"{label} does not exist: {path}")
    if args.member < 0:
        raise SystemExit("--member must be non-negative")
    if args.num_members <= args.member:
        raise SystemExit("--num-members must be greater than --member")
    if args.cache_capacity is not None and args.cache_capacity < 0:
        raise SystemExit("--cache-capacity must be non-negative")
    if args.threads is not None and args.threads <= 0:
        raise SystemExit("--threads must be positive")
    if not args.set_name or "/" in args.set_name or "\\" in args.set_name:
        raise SystemExit("--set-name must be a non-empty directory-safe name")

    native_checkpoint = _checkpoint_from_native(args.native)
    hybrid_checkpoint = _checkpoint_from_hybrid(args.hybrid)
    if native_checkpoint != hybrid_checkpoint:
        raise SystemExit("native model and hybrid artifact reference different checkpoints")

    set_directory = args.output_root / args.set_name
    member_stem = f"{args.set_name}_{args.member:04d}"
    info_target = set_directory / f"{args.set_name}.info"
    hybrid_target = set_directory / f"{member_stem}.dat"
    native_target = set_directory / f"{member_stem}.native"
    targets = (info_target, hybrid_target, native_target)
    existing = [path for path in targets if path.exists()]
    if existing and not args.force:
        names = ", ".join(str(path) for path in existing)
        raise SystemExit(f"output already exists (pass --force to replace): {names}")

    set_directory.mkdir(parents=True, exist_ok=True)
    info = args.source_info.read_text()
    info = _replace_yaml_scalar(info, "Format", FORMAT_NAME)
    info = _replace_yaml_scalar(info, "NumMembers", str(args.num_members))
    info = _replace_yaml_scalar(info, "HybridArtifactFormat", '"DPD-HYBRID-B1"')
    info = _replace_yaml_scalar(info, "HybridNativeFormat", '"DPDNAT1"')
    if args.cache_capacity is not None:
        info = _replace_yaml_scalar(info, "HybridCacheCapacity", str(args.cache_capacity))
    if args.threads is not None:
        info = _replace_yaml_scalar(info, "HybridThreads", str(args.threads))

    _atomic_copy(args.hybrid, hybrid_target)
    _atomic_copy(args.native, native_target)
    _atomic_text(info, info_target)

    print(f"PDFxTMD hybrid set: {set_directory}")
    print(f"  metadata: {info_target.name}")
    print(f"  hybrid artifact: {hybrid_target.name} ({hybrid_target.stat().st_size} bytes)")
    print(f"  neural model: {native_target.name} ({native_target.stat().st_size} bytes)")
    print(f"  checkpoint sha256: {native_checkpoint.hex()}")


if __name__ == "__main__":
    main()
