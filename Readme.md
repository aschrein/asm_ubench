## Description
A simple framework to benchmark x86 assembly on linux  

## Build
```console
## Arch linux setup
## don't forget to install linux-headers
cd 3rdparty/libpfc
make
sudo echo 0 > /proc/sys/kernel/nmi_watchdog
sudo echo 2 > /sys/bus/event_source/devices/cpu/rdpmc
sudo insmod pfc.ko
cd ../../
mkdir build
cd build
cmake ../
make
```