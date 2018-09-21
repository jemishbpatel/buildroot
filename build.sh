make clean
make raspberrypi0w_defconfig
make
rm output/images/sdcard.img
make emtguiapp
make
