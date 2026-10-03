#!/usr/bin/env python3
"""Install an APK and verify offline catalog extraction and relaunch on adb."""

import argparse
import hashlib
import shlex
import subprocess
import time
import sys
from pathlib import Path

PACKAGE = "com.leomoon.ParsiNama"


def run(*args: str, timeout: int = 120) -> str:
    result = subprocess.run(args, check=True, text=True, capture_output=True, timeout=timeout)
    return result.stdout.strip()


def adb_shell(command: str) -> str:
    return run("adb", "shell", command)


def private(command: str) -> str:
    return adb_shell(f"run-as {PACKAGE} {command}")


def launch() -> None:
    output = adb_shell(f"monkey -p {PACKAGE} -c android.intent.category.LAUNCHER 1")
    if "Events injected: 1" not in output:
        raise RuntimeError(f"App launch failed: {output}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("apk", type=Path)
    parser.add_argument("catalog", type=Path)
    args = parser.parse_args()
    expected_size = args.catalog.stat().st_size
    with args.catalog.open("rb") as catalog:
        expected_hash = hashlib.file_digest(catalog, "sha256").hexdigest()

    print(run("adb", "install", "-r", str(args.apk), timeout=900), flush=True)
    adb_shell("svc wifi disable")
    adb_shell("svc data disable")
    launch()

    deadline = time.monotonic() + 900
    path = ""
    while time.monotonic() < deadline:
        found = private("find . -name parsinama-catalog.sqlite")
        if found:
            path = found.splitlines()[0]
            size = int(private(f"stat -c %s {shlex.quote(path)}"))
            marker = private("find . -name parsinama-catalog.sqlite.sha256")
            if size == expected_size and marker:
                break
        time.sleep(5)
    else:
        raise TimeoutError("First launch did not create the complete private catalog")

    sidecar = private(f"cat {shlex.quote(path + '.sha256')}").strip()
    assert sidecar == expected_hash, "Installed catalog checksum marker does not match APK catalog"
    first_timestamp = private(f"stat -c %Y {shlex.quote(path)}")
    print(f"Fresh offline launch installed {expected_size:,} catalog bytes at {path}", flush=True)

    time.sleep(2)
    adb_shell(f"am force-stop {PACKAGE}")
    launch()
    time.sleep(15)
    second_timestamp = private(f"stat -c %Y {shlex.quote(path)}")
    assert second_timestamp == first_timestamp, "Relaunch unexpectedly recopied the catalog"
    assert adb_shell(f"pidof {PACKAGE}"), "App stopped after relaunch"
    print("Offline relaunch reused the installed catalog", flush=True)


if __name__ == "__main__":
    try:
        main()
    except Exception:
        print(run("adb", "logcat", "-d", "-t", "200"), file=sys.stderr)
        raise
