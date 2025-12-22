.PHONY: run buddha-reply analysis clean

MODE ?= release

ifeq ($(MODE), debug)
	CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=Debug
else
	CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=Release
endif

all: buddha-reply

build:
	cmake . -B build $(CMAKE_FLAGS)

buddha-reply: build
	make -C build

analysis:
	./scripts/analysis.py

run: ./build/buddha-reply
	./build/buddha-reply

clean:
	rm -rf ./build
	rm -rf ./data/analysis
