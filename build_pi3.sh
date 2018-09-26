make clean
make raspberrypi3_defconfig
make
rm output/images/sdcard.img
make emtguiapp
make
