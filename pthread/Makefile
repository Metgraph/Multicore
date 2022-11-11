BUILD_DIR ?= build
MPICC ?= mpicc
BIN = p10/pa4.1.c

.PHONY: all clean

all: ${BIN}

${BIN}:
	[ -d "build/$(@D)" ] || mkdir build/$(@D)
	${MPICC} -o build/$@ $@.c

clean:
	rm -rf build/*
