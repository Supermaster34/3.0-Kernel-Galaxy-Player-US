#!/bin/sh
adb shell reboot download
sleep 2
heimdall flash --kernel arch/arm/boot/zImage
