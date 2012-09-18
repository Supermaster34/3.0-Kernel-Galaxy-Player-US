#!/bin/sh
adb shell reboot download
sleep 7
heimdall flash --kernel arch/arm/boot/zImage
