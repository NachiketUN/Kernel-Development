# sudo mount -o loop dev_kernel_grub.img /mnt/floppy
# sudo cp kernel.bin /mnt/floppy/
# sleep 1s
# sudo umount /mnt/floppy/
mkdir ./floppy
hdiutil attach mp1.img -mountpoint ./floppy
cp kernel.elf ./floppy
sleep 1
hdiutil detach ./floppy
rmdir ./floppy
