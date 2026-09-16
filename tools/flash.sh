#!/bin/bash

DEVICE_PORT="/dev/ttyACM0"  # Adjust this to your device port

REPO_ROOT="$(git rev-parse --show-toplevel)"
cd "${REPO_ROOT}/button_hub" &&
arduino-cli upload -p "${DEVICE_PORT}" --fqbn m5stack:esp32:m5stack_core2 --input-dir "${REPO_ROOT}/_build" 
