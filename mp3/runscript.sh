#rm -f *.o *.bin
make -f makefile
bash copykernel.sh
bochs -q -f bochsrc.bxrc