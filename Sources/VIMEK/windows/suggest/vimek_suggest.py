# Copyright (C) 2026 GrazT. SPDX-License-Identifier: GPL-3.0-only
"""Entry point of vimek-suggest.exe, the bundled suggestion server.

VIMEK starts it when "Gợi ý dấu bằng AI" is on. It runs `virestore.serve` from
vi-restore-llm on onnxruntime only (no PyTorch), with the models next to the exe:

    models/tagger/    config.json, tagger.int8.onnx
    models/boundary/  config.json, lexicon.json, boundary.int8.onnx

Arguments are passed to virestore.serve after these defaults, so `--port` and `--threads` can be
overridden. Built by scripts/build-suggest-server.ps1.
"""

import os
import sys


def main() -> None:
    here = os.path.dirname(os.path.abspath(sys.executable if getattr(sys, "frozen", False) else __file__))
    models = os.path.join(here, "models")
    sys.argv = [sys.argv[0],
                "--model", "tagger-onnx", "--tagger", os.path.join(models, "tagger"), "--onnx-file", "tagger.int8.onnx",
                "--boundary", os.path.join(models, "boundary"), "--boundary-onnx", "boundary.int8.onnx",
                "--threads", str(min(4, os.cpu_count() or 1)), "--stdlib",
                *sys.argv[1:]]
    from virestore.serve import main as serve

    serve()


if __name__ == "__main__":
    main()
