# Use the latest version of Ubuntu as the base image
FROM ubuntu:latest

# Set the maintainer label
LABEL AUTHORS="Ian Yurychuk"


# Set environment variables to non-interactive (this will prevent some prompts)
ENV DEBIAN_FRONTEND=non-interactive


# This file was created basd on the "Install the SDK" section of
# the Getting Started guide for the STM32MP157x-DK2
# https://wiki.st.com/stm32mpu/wiki/Getting_started/STM32MP1_boards/STM32MP157x-DK2/Develop_on_Arm%C2%AE_Cortex%C2%AE-A7/Install_the_SDK


# Update package lists, install packages required by OpenEmbedded/Yocto
RUN apt-get update -y && \
    apt-get install -y --no-install-recommends \
    gawk \
    wget \
    git \
    diffstat \
    unzip \
    texinfo \
    gcc-multilib \
    chrpath \
    socat \
    cpio \
    python3 \
    python3-pip \
    python3-pexpect \
    xz-utils \
    debianutils \
    iputils-ping \
    python3-git \
    python3-jinja2 \
    libegl1-mesa \
    libsdl1.2-dev \
    pylint \
    xterm \
    bsdmainutils

# Install packages needed by some Developer Package use cases
RUN apt-get update -y && \
    apt-get install -y --no-install-recommends \
    build-essential \
    libncurses-dev \
    libyaml-dev \
    libssl-dev 


RUN apt update -y && \
    apt install -y --no-install-recommends \
    python-is-python3


# Setup a tool directory in /home/dev and download the repo client
# repo is used for downloading the Distribution Package
WORKDIR /home/dev

RUN apt-get update -y && \
    apt-get install -y --no-install-recommends repo

RUN mkdir WORKING_DIRECTORY && \
    cd WORKING_DIRECTORY && \
    repo init -b main -u https://android.googlesource.com/platform/manifest

# Increase MMC partitions to 16 (default is 8, ST packages need >10)
RUN mkdir -p /etc/modprobe.d && \
    echo 'options mmc_block perdev_minors=16' > /tmp/mmc_block.conf && \
    mv /tmp/mmc_block.conf /etc/modprobe.d/mmc_block.conf


# Download and install the Developer Package

# The developer package cannot be downloaded from ST.com through
# the dockerfile, because login credentials are required and the 
# the download link might also expire
# Instead, we must copy the developer package archive from our host
# machine. Pass the SDK_PACK argument into the docker build using
# the --build-arg option.
# EG.
# docker build --build-arg SDK_PATH=/home/ian/Downloads/en.SDK-x86_64-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar.gz

RUN mkdir -p /home/dev/STM32MPU_workspace/tmp

# Define a build-time argument for the SDK file path
ARG SDK_PATH

COPY ${SDK_PATH} /home/dev/STM32MPU_workspace/tmp

WORKDIR /home/dev/STM32MPU_workspace/tmp

RUN apt-get update && apt-get install -y gzip

# the regular tar xvf command was failing, run through gzip first

RUN gzip -cd en.SDK-x86_64-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar.gz > en.SDK-x86_64-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar

RUN tar xvf en.SDK-x86_64-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar

# file was missing, install sed and bash for good measure. flex and bison needed for make multi_v7_defconfig
RUN apt-get update && apt-get install -y file sed bash

RUN mkdir -p /home/dev/STM32MPU_workspace/STM32MP1-Ecosystem-v5.0.0/Developer-Package/SDK && \
    chmod +x stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21/sdk/st-image-weston-openstlinux-weston-stm32mp1-x86_64-toolchain-4.2.1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.sh && \
    ./stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21/sdk/st-image-weston-openstlinux-weston-stm32mp1-x86_64-toolchain-4.2.1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.sh -d /home/dev/STM32MPU_workspace/STM32MP1-Ecosystem-v5.0.0/Developer-Package/SDK

# before trying to compile, run the environment script:
# source SDK/environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi
# then confirm environment

# echo $ARCH
# arm
#
# echo $CROSS_COMPILE
# arm-ostl-linux-gnueabi-
#
# $CC --version
# arm-ostl-linux-gnueabi-gcc (GCC) 12.2.0
# Copyright (C) 2022 Free Software Foundation, Inc.
# This is free software; see the source for copying conditions.  There is NO
# warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# echo $OECORE_SDK_VERSION
# 4.2.1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21



ARG SRC_PATH

COPY ${SRC_PATH} /home/dev/STM32MPU_workspace/tmp

# the regular tar xvf command was failing, run through gzip first

RUN gzip -cd en.sources-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar.gz > en.sources-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar

RUN tar xvf en.sources-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar

# Extract Linux source code
RUN cd stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21/sources/arm-ostl-linux-gnueabi/linux-stm32mp-6.1.28-stm32mp-r1-r0 && \
    tar xvf linux-6.1.28.tar.xz && \
    # apply ST patches
    pwd && \
    cd linux-6.1.28 && \
    pwd && \
    for p in `ls -1 ../*.patch`; do patch -p1 < $p; done

RUN apt-get update && apt-get install -y flex bison bc


# apply fragments
RUN cd stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21/sources/arm-ostl-linux-gnueabi/linux-stm32mp-6.1.28-stm32mp-r1-r0/linux-6.1.28 && \
    make ARCH=arm multi_v7_defconfig "fragment*.config" && \
    for f in `ls -1 ../fragment*.config`; do scripts/kconfig/merge_config.sh -m -r .config $f; done && \
    yes '' | make ARCH=arm oldconfig

# Build Linux source code
# Build kernel images (uImage and vmlinux) and device tree (dtbs)
SHELL ["/bin/bash", "-c"]

RUN cd /home/dev/STM32MPU_workspace/STM32MP1-Ecosystem-v5.0.0/Developer-Package && \
    # /bin/bash -c "source SDK/environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi" && \
    source SDK/environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi && \
    cd ../../tmp/stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21/sources/arm-ostl-linux-gnueabi/linux-stm32mp-6.1.28-stm32mp-r1-r0/linux-6.1.28 && \
    make ARCH=arm uImage vmlinux dtbs LOADADDR=0xC2000040 && \
    # Build kernel module
    make ARCH=arm modules && \
    # Generate output build artifacts
    mkdir -p $PWD/install_artifact/ && \
    make ARCH=arm INSTALL_MOD_PATH="$PWD/install_artifact" modules_install

# WORKDIR /home/dev/STM32MPU_workspace/STM32MP1-Ecosystem-v5.0.0/Developer-Package
