#!/bin/bash

set -eux

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  echo "Usage: $0 [OTA_ENDPOINT] [OTA_LABEL]"
  echo "  OTA_ENDPOINT: OTA endpoint URL (default: empty)"
  echo "  OTA_LABEL: OTA label (default: empty)"
  echo "Example: $0 https://example.com/v1 stg"
  exit 0
fi

OTA_ENDPOINT="${1:-}"
OTA_LABEL="${2:-}"

OUTPUT_DIR="$(dirname $0)/../_build"
DOCKER_IMAGE=pfr-kachaka-button

VERSION="$(git describe --tags --always --dirty)"
docker build -t ${DOCKER_IMAGE} .

rm -rf "${OUTPUT_DIR}"
mkdir "${OUTPUT_DIR}"
docker run --rm -v "$(realpath $OUTPUT_DIR):/workspace/button_hub/.build" \
  -e OTA_ENDPOINT="${OTA_ENDPOINT}" \
  -e OTA_LABEL="${OTA_LABEL}" \
  "${DOCKER_IMAGE}" make build VERSION="${VERSION}"

ls -la "${OUTPUT_DIR}"/*
