#!/bin/sh
export PATH=/home/jonathan/jbandroid/system/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.6/bin
PATH=$PATH:/home/jonathan/tools/sdk/platform-tools:/home/jonathan/tools/java6/bin:/usr/lib/lightdm/lightdm:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
make CROSS_COMPILE=arm-linux-androideabi- ARCH=arm clean
