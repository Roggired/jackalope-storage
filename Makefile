BUILD_DIR=build

all: build

test: build
	cd ${BUILD_DIR}/bin; \
		./test_example_unit_test

build:
	mkdir -p ${BUILD_DIR};\
	cd ${BUILD_DIR};\
	cmake ..;\
	make -j16

clean:
	cd ${BUILD_DIR} || exit 0;\
	make clean
	rm -rf ${BUILD_DIR}

clang-format:
	find . -name '*.cpp' -o -name '*.hpp' -exec clang-format --style=Google -i \{\} \;

