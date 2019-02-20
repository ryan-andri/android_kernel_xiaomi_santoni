# AnyKernel2 Ramdisk Mod Script
# osm0sis @ xda-developers

## AnyKernel setup
# begin properties
properties() { '
do.devicecheck=1
do.modules=0
do.cleanup=1
do.cleanuponabort=1
device.name1=santoni
device.name2=Xiaomi
device.name3=Redmi 4X
'; } # end properties

# shell variables
block=/dev/block/bootdevice/by-name/boot;
is_slot_device=0;
ramdisk_compression=auto;


## AnyKernel methods (DO NOT CHANGE)
# import patching functions/variables - see for reference
. /tmp/anykernel/tools/ak2-core.sh;


## AnyKernel file attributes
# set permissions/ownership for included ramdisk files
# set permissions for included ramdisk files
chmod -R 755 $ramdisk
chmod +x $ramdisk/sbin/spa


## AnyKernel install
dump_boot;

# begin ramdisk changes

# fstab.qcom
if [ -f /fstab.qcom ]; then
insert_line fstab.qcom "data        f2fs" before "data        ext4" "/dev/block/bootdevice/by-name/userdata     /data        f2fs    nosuid,nodev,noatime,inline_xattr,data_flush      wait,check,encryptable=footer,formattable,length=-16384";
insert_line fstab.qcom "cache        f2fs" after "data        ext4" "/dev/block/bootdevice/by-name/cache     /cache        f2fs    nosuid,nodev,noatime,inline_xattr,flush_merge,data_flush wait,formattable,check";
fi;
backup_file init.rc;
grep "import /init.spectrum.rc" init.rc >/dev/null || sed -i '1,/.*import.*/s/.*import.*/import \/init.spectrum.rc\n&/' init.rc
	
# end ramdisk changes

write_boot;

## end install

# Add empty profile locations
		if [ ! -d /data/media/Spectrum ]; then
		  ui_print " "; ui_print "Creating /data/media/0/Spectrum...";
		  mkdir /data/media/0/Spectrum;
		fi
		if [ ! -d /data/media/Spectrum/profiles ]; then
		  mkdir /data/media/0/Spectrum/profiles;
		fi
		if [ ! -d /data/media/Spectrum/profiles/*.profile ]; then
		  ui_print " "; ui_print "Creating empty profile files...";
		  touch /data/media/0/Spectrum/profiles/balance.profile;
		  touch /data/media/0/Spectrum/profiles/performance.profile;
		  touch /data/media/0/Spectrum/profiles/battery.profile;
		  touch /data/media/0/Spectrum/profiles/gaming.profile;
		fi
		
		