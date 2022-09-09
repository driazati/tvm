#!/usr/bin/env python
import sys
from pathlib import Path
import shutil

with open("out.log", "w") as f:
    def fprint(*args, **kwargs):
        print(*args, file=f)
    
    fprint(sys.argv)
    for item in sys.argv:
        if item.startswith("/") and "gobbler" not in item:
            p = Path(item)
            if p.exists():
                shutil.copy(p, "out-" + p.name)