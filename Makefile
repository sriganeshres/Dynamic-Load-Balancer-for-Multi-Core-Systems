#!/bin/bash

.PHONY: all clean build rebuild docs test install

all: build

build:
	mkdir -p build
	cd build && cmake .. && make

clean:
	rm -rf build

rebuild: clean build

docs:
	cd build && make docs

test:
	cd build && ctest --output-on-failure

install:
	cd build && sudo make install

