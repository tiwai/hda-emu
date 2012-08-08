#!/bin/sh
find ../codecs/canonical/ -type f | sort | xargs -n1 ./hda-emu-tester.py --file

