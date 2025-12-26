import subprocess
from pathlib import Path

CHUNG_PATH = Path() / "build" / "chung"
COMPILED_PATH = Path() / "chungbuild" / "output.out"

def run_program(path: Path, *args):
    result = subprocess.run([path, *args], capture_output=True, text=True, timeout=5, encoding="latin-1")

    return result.stdout, result.stderr, result.returncode

def compile(path: str):
    stdout, stderr, returncode = run_program(CHUNG_PATH, "parse", path)
    assert returncode == 0, "Chunglang compiler failed with nonzero exit code"
    return stdout, stderr, returncode

def run_compiled_program():
    return run_program(COMPILED_PATH)
