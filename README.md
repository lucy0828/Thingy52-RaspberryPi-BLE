# Thingy52-RaspberryPi-BLE
This project is based on [labapart gattlib](https://github.com/labapart/gattlib) to connect Thingy52 and Raspberry Pi.

## Build GattLib

* Gattlib requires the following packages: `libbluetooth-dev`, `libreadline-dev`.  
On Debian based system (such as Ubuntu), you can installed these packages with the
following command: `sudo apt install libbluetooth-dev libreadline-dev`
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
cd <notification_sync-root>
mkdir build && cd build
cmake ..
make
./notification_maf
```
