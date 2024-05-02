# Kachaka Button Hub

The "Kachaka Button Hub" is software for IoT devices that converts [BLE beacon](https://www.braveridge.com/product/archives/22) events into commands for [Kachaka](https://kachaka.life/). It operates on the [M5Stack v2.7](https://docs.m5stack.com/en/core/basic_v2.7) platform and the software is completely open-source. The system comprises two main components: one that translates BLE iBeacon signals into commands for the [Kachaka API](https://github.com/pf-robotics/kachaka-api) via gRPC, and another that allows for configuration through a Web UI.

## Build Instructions

### Prerequisites

These instructions are designed for Ubuntu or Ubuntu-based distributions. They focus on using the `apt-get` command for installing necessary tools.

Ensure you have the following tools and libraries installed:

- GNU Make
    - Install with the command: `sudo apt-get install make`
- Protocol Buffers Compiler (protoc)
    - Install with the command: `sudo apt-get install protobuf-compiler`
- Python Virtual Environment and pyserial
    - Install with the command: `sudo apt-get install python3-serial python3-venv`
- socat
    - Install with the command: `sudo apt-get install socat`
- npm (Node.js)
    - Download and install from the [Node.js website](https://nodejs.org/)
- Arduino CLI
    - Installation instructions are available in the [Arduino CLI documentation](https://arduino.github.io/arduino-cli/)

### Arduino Build Preparation

#### Update Submodules

First, update the git submodules:

```bash
git submodule update --init
```

#### Install Core and Libraries

Install the necessary core and libraries for Arduino:

```bash
arduino-cli core install m5stack:esp32 --additional-urls https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json
arduino-cli lib install ArduinoJson ESPAsyncWebServer ESPping M5Unified NimBLE-Arduino TickTwo
```

#### Add Nanopb to Arduino Library

Link the Nanopb library to your Arduino libraries directory:

```bash
cd ~/Arduino/libraries
ln -s ~/path/to/kachaka-button-hub/nanopb .
```

Replace `~/path/to/kachaka-button-hub` with the actual path to your cloned repository.

#### Prepare for Web UI Build

Set up the Web UI project:

```bash
cd ~/path/to/kachaka-button-hub/webui
npm ci
```

#### Add Permission to Serial Device

Add your user to the `dialout` group to grant permission to access the serial device. You may need to log out and then log back in for these changes to take effect:

```bash
sudo adduser $USER dialout
```

### Build and Flash

Navigate to the `button_hub` directory and run `make` to build, upload, and connect the serial monitor:

```bash
cd ~/path/to/kachaka-button-hub/button_hub
make
```

- This process includes building the web UI. The generated HTML, JavaScript, and other files will be incorporated into the program as data.
- A complete build process may take several minutes, depending on your system.

## Build using Docker

You can also build the project using Docker. This method is useful if you do not want to install the necessary tools and libraries on your system.
Install Docker on your system and run the following command:

```bash
git submodule update --init
./tools/build-by-docker.sh
```

This command will build the project and generate the binary in the `_build` directory.

To upload the binary to the M5Stack device, connect the device to your computer and run the following command:

```bash
docker run --rm \
    --device /dev/ttyACM0 \
    --volume ./_build:/_build \
    pfr-kachaka-button \
    arduino-cli upload -p /dev/ttyACM0 --fqbn m5stack:esp32:m5stack_core2 --input-file /_build/button_hub.ino.bin
```

---

**Note:** Replace `~/path/to/kachaka-button-hub` with the actual path where your Kachaka Button Hub repository is cloned to ensure all commands work correctly.
