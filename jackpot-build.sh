#!/bin/sh
#export PATH=/home/jonathan/jbandroid/system/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin
#PATH=$PATH:/home/jonathan/tools/sdk/platform-tools:/home/jonathan/tools/java6/bin:/usr/lib/lightdm/lightdm:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
mkdir -p kernel/usr/initramfs
rm -rf kernel/usr/initramfs/*
cp -R ../initramfs_yp-g70/* kernel/usr/initramfs
rm $(find kernel/usr/initramfs -name EMPTY_DIRECTORY -print)
rm -rf $(find kernel/usr/initramfs -name .git -print)
make -j1 CROSS_COMPILE=arm-eabi- ARCH=arm yp_g70_usa_defconfig
# get rid of the damn + after the version
export LOCALVERSION=""
export KBUILD_BUILD_HOST="ThinkPad-X220"
export KBUILD_BUILD_USER="JackpotClavin"
make -j1 CROSS_COMPILE=arm-eabi- ARCH=arm && cp -a $(find . -name *.ko -print |grep -v initramfs) kernel/usr/initramfs/lib/modules/ && make -j1 CROSS_COMPILE=arm-eabi- ARCH=arm
# adb shell reboot download
# sleep 5
# heimdall flash --kernel arch/arm/boot/zImage

#arm-eabi-
#arm-eabi-
