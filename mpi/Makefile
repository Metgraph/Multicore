BUILD_DIR ?= build
MPICC ?= mpicc
BIN = p5/ex3.2 p5/ex3.4 p5/pa3.1 p5/pa3.2 p5/pa3.3a p7/ex3.13 p7/ex3.20 p9/ex3.9 p9/exEa p9/exEb

.PHONY: all clean

all: ${BIN}

${BIN}:
	[ -d "build/$(@D)" ] || mkdir build/$(@D)
	${MPICC} -o build/$@ $@.c

clean:
	rm -rf build/*
