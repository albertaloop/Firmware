# Prerequisites

- Docker

- make

- STM32MP157f-DK2 evaluation board

- Micro USB cable

- Ethernet cable

- 5V, 3A USB-C Power Supply

- Serial terminal program (GTKTerm, TeraTerm, PuTTy, etc.)


# Building the code

## Step 1 - Build and run Docker container using Makefile

A Makefile is used for running Docker commands as phony Make targets.

There are three phony targets in the Makefile: docker_build, docker_run, and docker_run_hostv. We will only need docker_build and docker_run_hostv.

``$ make docker_build``

contains the following command:


```
	docker build \
    --build-arg SDK_PATH=en.SDK-x86_64-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar.gz \
    --build-arg SRC_PATH=en.sources-stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21.tar.gz \
    -t iany/state-machine .
```

The two build arguments allow us to copy zipped tar files into the container during the build. The
archives contain linux kernel sources, and the SDK for cross compilation. When you need to compile
a package from sources in a Dockerfile, you typically just download the archive using wget or curl.
However, these archives are provided from STM32. to downlaod these archives, you need to enter your STM32
credentials. It is possible for us to version control the credentials and try to login to their website
when we build the container, but that is complicated and might be against ST's policy for downloading
resources. Instead, I just decided we could download these archives ourselves. We could version control
the archives, but they are pretty large files so we cannot upload them on a free github account!

After you download these archives, place them in the same directory as the Dockerfile. Or you can modify the build args to the correct paths, whereever you have placed the archives on your system.


``$ make docker_run_hostv``


```
  sudo docker run --rm -it --privileged -v "$(pwd):/home/dev" iany/state-machine:latest bash
```

-i is "interactive" which keeps STDIN open even if not attached
-t is add a terminal

These two options together opens a terminal in the running container.

--rm Will remove the container when it container.

--privileged Give extended privileges to this container

-v will mount a directory on your machine to the docker container. In this command, the directory "/home/dev" on the
container will be the same as the directory from which you ran the command. This allows us to access our source files
from within the container.



## Step 2 - Build the kernel




## Step 3 - Build the executable


# Run the program


## Plug in the board and open a terminal

![image](https://github.com/user-attachments/assets/9b9fb41a-963c-4ca6-9a38-5a32e314a4ee)
<br>Image taken from STM32mp157x-DK2 [Getting Started Guide](https://wiki.stmicroelectronics.cn/stm32mpu/wiki/Getting_started/STM32MP1_boards/STM32MP157x-DK2/Let%27s_start/Unpack_the_STM32MP157x-DK2_board)

Plug in the USB-C Power Supply cable into the "Power Supply" port and the Micro USB cable into the "ST-Link/V2-1".


Open your serial terminal, and select the USB device that was just connected. Set baud rate to 115200. 
If you are using GTKTerm, you do this by clicking the "Configuration" menu, then clicking "Port"

![image](https://github.com/user-attachments/assets/4b8f7e3f-0afe-4fa5-af7c-08d5232bc4b8)
<br>
Configuring serial terminal with GTKTerm


Enable auto carriage return and line feed. On GTKTerm, this is done by clicking "Configuration" and "CR LF auto".


Press the black "reset" button on the board. You should see Kernel log messages appear as the board boots up. Eventually you will be presented with a Linux terminal.
<br>
![image](https://github.com/user-attachments/assets/54346a6e-a64d-4ea2-b05c-3a54303c556c)




## Copy the executable to the target

Open a terminal on the target using the UART over USB serial connection.

If the can0 interface is properly configured, and the hardware is connected, then the can0 interface should be available. You can check for can0 interface using ``$ ip link``


## Automatic start using systemd service

## Manual start

Assign static IP

`` $ ip addr 192.168.1.100/24 dev end0``


Bring the can0 interface up
```
$ ip link set can0 down
$ ip link set can0 type can bitrate 250000 fd off
$ ip link set can0 up
```
