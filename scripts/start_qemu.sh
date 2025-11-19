qemu-system-arm -M vexpress-a9 -m 1024M -smp 4 \
  -kernel /home/zayn/project/Embedded-System-Design/assets/zImage_5.15 \
  -dtb /home/zayn/project/Embedded-System-Design/assets/vexpress-v2p-ca9.dtb \
  -append "root=/dev/mmcblk0 rw rootfstype=ext4 init=/linuxrc console=ttyAMA0 ip=10.0.0.2:10.0.0.1:10.0.0.1:255.255.255.0" \
  -drive if=sd,file=/home/zayn/project/Embedded-System-Design/assets/rootfs.ext4,format=raw \
  -net nic -net tap,ifname=tap0,script=no,downscript=no \
  -nographic
