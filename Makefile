name = cerbtora

MAKEFLAGS += --no-print-directory
all: build/Makefile
	@cmake --build build --parallel
	@cmake --install build --prefix .
	@rm -rf include lib
build/Makefile: CMakeLists.txt
	cmake -DCMAKE_BUILD_TYPE=Release -DSTATIC=ON -DTOOLS=ON -B build
container: clean
	podman build -t $(name) .
	podman create --name $(name) $(name)
	podman cp $(name):/usr/local/bin .
	podman rm $(name)
clean:
	rm -rf build bin
	-podman rmi $(name)
.PHONY: all container clean
