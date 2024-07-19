# Prerequisites

Docker

make

# Step 1 - Build and run Docker container using Makefile

A Makefile is used for running Docker commands as phony Make targets.

There are three phony targets, docker_build, docker_run, and docker_run_hostv. We will only need docker_build and docker_run_hostv.

``$ make docker_build``

contains the following command:


```
		docker build \
    --build-arg SDK_PATH=en.SDK-x86_64-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar.gz \
    --build-arg SRC_PATH=en.sources-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar.gz \
    -t iany/state-machine .
```

The two build arguments allow us to copy zipped tar files into the container during the build. The archives contain linux kernel sources, and the SDK for cross compilation. When you need to compile a package from sources in a Dockerfile, you typically just download the archive using wget or curl. However, these archives are provided from STM32. to downlaod these archives, you need to enter your STM32 credentials. It is possible for us to version control the credentials and try to login to their website when we build the container, but that is complicated and might be against ST's policy for downloading resources. Instead, I just decided we could download these archives ourselves. We could version control the archives, but they are pretty large files so we cannot upload them on a free github account!

After you download these archives, place them in the same directory as the Dockerfile. Or you can modify the build args to the correct paths, whereever you have placed the archives on your system.


$ make docker_run_hostv