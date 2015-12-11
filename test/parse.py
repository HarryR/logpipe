from __future__ import print_function
import json
import sys

try:
    for line in sys.stdin:
        try:
            row = json.loads(line)
        except Exception:
            sys.stderr.write(line)
        sys.stdout.write(line)
except:
    pass
