# Prerequisites

- Docker

- make

- STM32MP157f-DK2 evaluation board

- Micro USB cable

- Ethernet cable

- 5V, 3A USB-C Power Supply

- Serial terminal program (GTKTerm, TeraTerm, PuTTy, etc.)

<br><br>


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


<br><br>

## Step 2 - Build the kernel

<br><br>



## Step 3 - Build the executable

<br><br>

# Run the program


## Plug in the board and open a terminal

![image](https://github.com/user-attachments/assets/9b9fb41a-963c-4ca6-9a38-5a32e314a4ee)
<br>Image taken from STM32mp157x-DK2 [Getting Started Guide](https://wiki.stmicroelectronics.cn/stm32mpu/wiki/Getting_started/STM32MP1_boards/STM32MP157x-DK2/Let%27s_start/Unpack_the_STM32MP157x-DK2_board)

- Plug in the USB-C Power Supply cable into the "Power Supply" port and the Micro USB cable into the "ST-Link/V2-1".

<br>

- Open your serial terminal, and select the USB device that was just connected. Set baud rate to 115200. 
If you are using GTKTerm, you do this by clicking the "Configuration" menu, then clicking "Port"

<br>


![image](https://github.com/user-attachments/assets/4b8f7e3f-0afe-4fa5-af7c-08d5232bc4b8)
<br>
Configuring serial terminal with GTKTerm

<br>

- Enable auto carriage return and line feed. On GTKTerm, this is done by clicking "Configuration" and "CR LF auto".

<br>

- Reset the board by pressing the black "reset" button. You should see Kernel log messages appear as the board boots up. Eventually you will be presented with a Linux terminal.

<br>
![image](https://github.com/user-attachments/assets/54346a6e-a64d-4ea2-b05c-3a54303c556c)


<br>

## Set up SSH connection from Host to Target

Open a terminal on the target using the UART over USB serial connection.

To copy files from you host machine to the target, you need to set up an SSH connection.

- Connect the board to your host computer using an Ethernet cable.

<br>

- Assign an IPv4 address to the ethernet interface on your host. Since we are creating a network over LAN, we will use the subnet 192.168.x.x. You also need to specify the subnet mask, which is the number of bits for the address portion of the IP address.

  `` $ sudo ip addr add 192.168.1.11/24 dev enp3s0``

  The ethernet interface for the host machine in this example is "enp3s0". You can check the name of your ethernet interface using  ``$ ip link``

  Here we used /24 as the subnet mask, which is equivalent to 255.255.255.0 (each field of the IPv4 address is a number between 0 and 255, so the address has 8 bits per field). The last 8 bits are used to identify individual hosts on the network. This means we can only communicate with devices that have an address starting with 192.168.1.x.

<br>

- Assign an IP address to the ethernet interface on the board.

  ``root@stm32mp1:~# ip addr add 192.168.1.10 dev end0``


- Generate a public key on the host using ssh-keygen, and add the key to the "authorized keys" file on the target.
  I do not remember how I did this before... I do know that the taget has "dropbear" installed for ssh.

<br>

## Copy the kernel image to the target

On the host, run the script "copy-to-host.sh"

``$ chmod u+x copy-to-host.sh``

``$ ./copy-to-host.sh``

<br>


## Copy the executable to the target

On the host run the script "copy-to-target.sh"

``$ chmod u+x copy-to-host.sh``

``$ ./copy-to-target.sh``

<br>


## Check CANBus availability

If the can0 interface is properly configured, and the hardware is connected, then the can0 interface should be available. You can check for can0 interface using ``$ ip link``


## Manually set up CAN interface and start program


- Make sure the podcan service is not running

  ``root@stm32mp1:~# systemctl stop podcan.service``

<br>

- Bring the can0 interface up
  ```
  root@stm32mp1:~# ip link set can0 down
  root@stm32mp1:~# ip link set can0 type can bitrate 250000 fd off
  root@stm32mp1:~# ip link set can0 up
  ```

  <br>

- Run candump utility in the background to see if CAN traffic is actually working.

  ``root@stm32mp1:~# candump can0 &``
  
  <br>

- Start the program

  ``root@stm32mp1:~# ./main``

## Automatic start using systemd service

- Check if the podcan service is running

  ```
  root@stm32mp1:~# systemctl is-enabled podcan.service
  enabled
  ``` 