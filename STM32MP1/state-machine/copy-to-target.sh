scp uImage root@192.168.1.10:/boot
scp stm32mp157f-dk2.dtb root@192.168.1.10:/boot
scp -r modules/* root@192.168.1.10:/lib/modules

# regenerate the list of module dependencies (modules.dep) and
# the list of symbols provided by modules (modules.symbols)
ssh root@192.168.1.10 '/sbin/depmod -a'
# Synchronize data on disk with memory
ssh root@192.168.1.10 'sync'
# Reboot the board
ssh root@192.168.1.10 'reboot'
