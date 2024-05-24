rm /home/app/uImage
rm /home/app/stm32mp157f-dk2.dtb
rm -r /home/app/modules

cp /home/dev/STM32MPU_workspace/tmp/stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21/sources/arm-ostl-linux-gnueabi/linux-stm32mp-6.1.28-stm32mp-r1-r0/linux-6.1.28/arch/arm/boot/uImage /home/app/

cp /home/dev/STM32MPU_workspace/tmp/stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21/sources/arm-ostl-linux-gnueabi/linux-stm32mp-6.1.28-stm32mp-r1-r0/linux-6.1.28/arch/arm/boot/dts/stm32mp157f-dk2.dtb /home/app/

rm /home/dev/STM32MPU_workspace/tmp/stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21/sources/arm-ostl-linux-gnueabi/linux-stm32mp-6.1.28-stm32mp-r1-r0/linux-6.1.28/install_artifact/lib/modules/6.1.28/source

rm /home/dev/STM32MPU_workspace/tmp/stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21/sources/arm-ostl-linux-gnueabi/linux-stm32mp-6.1.28-stm32mp-r1-r0/linux-6.1.28/install_artifact/lib/modules/6.1.28/build

# Need to source cross compile toolchain for specific $STRIP command that knows about kernel objects and '--strip-debug' option.
source /home/dev/STM32MPU_workspace/STM32MP1-Ecosystem-v5.0.0/Developer-Package/SDK/environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi


find /home/dev/STM32MPU_workspace/tmp/stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21/sources/arm-ostl-linux-gnueabi/linux-stm32mp-6.1.28-stm32mp-r1-r0/linux-6.1.28/install_artifact/ -name "*.ko" | xargs $STRIP --strip-debug --remove-section=.comment --remove-section=.note --preserve-dates

cp -r /home/dev/STM32MPU_workspace/tmp/stm32mp1-openstlinux-6.1-yocto-mickledore-mp1-v23.06.21/sources/arm-ostl-linux-gnueabi/linux-stm32mp-6.1.28-stm32mp-r1-r0/linux-6.1.28/install_artifact/lib/modules/ /home/app/
