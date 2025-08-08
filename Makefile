MAKEFLAGS += --no-print-directory
all: build/Makefile
	@cmake --build build --parallel
	@cmake --install build --prefix .
	@rm -rf include lib
build/Makefile: CMakeLists.txt
	cmake -DCMAKE_BUILD_TYPE=Release -DSTATIC=ON -DTOOLS=ON -B build
docker: clean
	docker build -t certifaiger .
	docker run --rm -it certifaiger examples/uniqueness_model.aag examples/uniqueness_witness.aag --qbf
clean:
	rm -rf build bin
.PHONY: all docker clean
