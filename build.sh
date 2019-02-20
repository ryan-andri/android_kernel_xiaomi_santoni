#!/bin/bash
make clean distclean mrproper
export ARCH=arm64
export CROSS_COMPILE=/root/uber/bin/aarch64-linux-android-
export KBUILD_BUILD_USER=magchuz
export KBUILD_BUILD_HOST=CrappyServer
time=$(date +%s)
rm -rf output
mkdir output
make -C $(pwd) O=output santoni_nontreble_defconfig
make -j2 -C $(pwd) O=output
cp output/arch/arm64/boot/Image.gz-dtb AnyKernel2/zImage
cd AnyKernel2
rm -rf *.zip
zip -r9 CrappyKernel-WineX-$time.zip * -x README.md CrappyKernel-WineX-$time.zip
curl -F chat_id="-1001324692867" -F document=@"CrappyKernel-WineX-$time.zip" https://api.telegram.org/bot757761074:AAFKxcBRT-hsNfyC0wXTH_GXJozT7yzflKU/sendDocument
cd ..
