#!/bin/bash

set -x

ampy -p /dev/ttyUSB3 put espSide.py
ampy -p /dev/ttyUSB3 put main.py
