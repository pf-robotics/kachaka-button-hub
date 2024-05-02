BOARD=m5stack:esp32:m5stack_core2
CXX_FLAGS=--build-property compiler.cpp.extra_flags="-DKB_M5STACK -DOTA_ENDPOINT=\"$(OTA_ENDPOINT)\" -DOTA_LABEL=\"$(OTA_LABEL)\""
# ~/.arduino15/packages/m5stack/hardware/esp32/2.1.0/tools/partitions/default_16MB.csv
PARTITION=default_16MB
APP_PARTITION_SIZE=6553600  # 0x640000

SKETCH_NAME = $(notdir $(shell pwd))


DEVICE=/dev/ttyACM0

DATA_GENERATED_FILES = data_index.css.cpp data_index.html.cpp data_index.js.cpp data_index.js.gz.cpp data_license.txt.gz.cpp
PB_GENERATED_FILES = kachaka-api.pb.c kachaka-api.pb.h


all: build upload serial

../webui/dist/license.txt.gz: ../webui/dist/license.txt
	gzip -c $^ > $@

data_%.cpp: ../webui/dist/%
	../tools/gen_cpp_data_code.py $^ $@

../webui/dist/%: ../webui/src/*
	cd ../webui && npm run build

build: $(DATA_GENERATED_FILES) $(PB_GENERATED_FILES)
	echo "constexpr char kVersion[] = \"$(shell git describe --tags --always --dirty)\";" > version.hpp
	-@rm -f compile_commands.json
	arduino-cli compile \
		--fqbn $(BOARD) \
		--build-property build.partitions=$(PARTITION) \
		--build-property upload.maximum_size=$(APP_PARTITION_SIZE) \
		--output-dir .build \
		$(CXX_FLAGS) \
		.
	$(MAKE) compile_commands.json

compile_commands.json:
	-@for dir in /tmp/arduino/sketches/*; do \
		test -f $$dir/$(SKETCH_NAME).ino.elf && ln -vs $$dir/compile_commands.json .; \
	done

upload:
	arduino-cli upload -p $(DEVICE) --fqbn $(BOARD) .

serial:
	socat $(DEVICE),b115200,raw,echo=0 -

updatepb: $(PB_GENERATED_FILES)

kachaka-api.pb.%: ../proto/kachaka-api.proto
	cp $^ ../nanopb
	rm -f ../nanopb/kachaka-api.options
	python3 -m venv ../nanopb/venv
	bash -c "source ../nanopb/venv/bin/activate; pip install -U pip; pip install "protobuf==3.20.3" grpcio-tools"
	bash -c "cd ../nanopb; source venv/bin/activate; protoc --plugin=protoc-gen-nanopb=./generator/protoc-gen-nanopb --nanopb_out=. kachaka-api.proto"
	mv ../nanopb/kachaka-api.pb.* .
	rm ../nanopb/kachaka-api.*
