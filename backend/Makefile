default_target: all

all:
	cmake --build build

BUILD_DIR = ./build/

ifeq ($(OS),Windows_NT)
	EXE_PATH = $(BUILD_DIR)Debug/lws-minimal-ws-server
else
	EXE_PATH = $(BUILD_DIR)lws-minimal-ws-server
endif

run:
	cmake --build build
	$(EXE_PATH)
