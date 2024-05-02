#!/bin/bash

set -eux

OUTPUT_DIR=_build
DOCKER_IMAGE=pfr-kachaka-button

docker build -t ${DOCKER_IMAGE} .

id=$(docker create "${DOCKER_IMAGE}" sleep 86400)
rm -rf "${OUTPUT_DIR}"
mkdir "${OUTPUT_DIR}"
docker cp "${id}:/workspace/button_hub/.build" - | tar -x -C "${OUTPUT_DIR}" --strip-components=1 -f -
docker rm -v "${id}" > /dev/null

ls -la "${OUTPUT_DIR}"/*
