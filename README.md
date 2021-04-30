# Thingy52-RaspberryPi-BLE
This project is based on [labapart gattlib](https://github.com/labapart/gattlib) to connect Thingy52 and Raspberry Pi.

## Prerequisition
You must download [Gattlib](https://github.com/labapart/gattlib) ARM 32-bit DEB 

## Build GattLib

* Gattlib requires the following packages: `libbluetooth-dev`, `libreadline-dev`.  
On Debian based system (such as Ubuntu), you can installed these packages with the
following command: `sudo apt install libbluetooth-dev libreadline-dev`
* In order to build your own code, copy CMakeLists.txt to your folder and edit into your build file name
* To build your code, follow the steps below
```
cd <gattlib-src-root>
mkdir build && cd build
cmake ..
make
```

## notification_sync
This project connects multiple Thingy52 with Raspberry Pi to get environment sensor notifications per 1 second.
```
cd <notification_sync-root>
mkdir build && cd build
cmake ..
make
./notification_sync
```

## notification_maf
This project connects multiple Thingy52 with Raspberry Pi to get level of humidity sensor using moving average filter.
```
cd <notification_maf-root>
mkdir build && cd build
cmake ..
make
./notification_maf
```
