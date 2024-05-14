.PHONY: docker_build docker_run docker_run_hostv

docker_build:
		docker build \
    --build-arg SDK_PATH=en.SDK-x86_64-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar.gz \
    --build-arg SRC_PATH=en.sources-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar.gz \
    -t iany/state-machine .

docker_run:
		docker run --rm -it --privileged iany/state-machine:latest bash

docker_run_hostv:
		docker run --rm -it --privileged \
		-v "$$(pwd):/home/app" \
		iany/state-machine:latest bash
