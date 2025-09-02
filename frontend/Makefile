BUILD_DIR = ./build

ifeq ($(OS), Windows_NT)
	EXE_PATH = $(BUILD_DIR)/Debug/im_c
else
	EXE_PATH = $(BUILD_DIR)/im_c
endif

.PHONY: configure build run clean all

configure:
	cmake -B $(BUILD_DIR) .

build: configure
	cmake --build $(BUILD_DIR)

run: build
	$(EXE_PATH)

all: run

clean:
	rm -rf $(BUILD_DIR)
