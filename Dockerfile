#=============================================================================
FROM ubuntu:22.04 as base

RUN apt-get update && apt-get install -y \
    curl \
    xz-utils \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://deb.nodesource.com/setup_20.x | bash - && apt-get install -y nodejs

RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
ENV PATH="/root/bin:${PATH}"
RUN arduino-cli core install --additional-urls https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json m5stack:esp32
RUN arduino-cli lib install \
        ArduinoJson \
        ESPAsyncWebServer \
        ESPping \
        M5Unified \
        NimBLE-Arduino \
        TickTwo

RUN apt-get update
RUN apt-get install -y \
        make \
        protobuf-compiler \
        python3-serial \
        python3-venv \
    && rm -rf /var/lib/apt/lists/*

#=============================================================================
FROM base as build

WORKDIR /workspace

COPY webui /workspace/webui
WORKDIR /workspace/webui
RUN npm ci

COPY button_hub /workspace/button_hub
COPY nanopb /workspace/nanopb
COPY proto /workspace/proto
COPY tools /workspace/tools
RUN ln -s /workspace/nanopb ~/Arduino/libraries/nanopb
WORKDIR /workspace/button_hub
RUN make build
