# magisk-cli
Magisk CLI interface

## What is it?

It's command-line interface for Magisk. You can manage modules (install, remove, turn off/on), manage rooted apps and patch boot image.

## Building

Firstly, you need cross-compilator, musl for static linkage and libsqlite3-dev. Then, run `make arm64exec` or `make armhfexec` for building magisk-cli.

## What can I do?

1. Manage root apps: delete app, add new app, enable/disable root access.
```
 ~ # /sbin/magisk-cli -r -h
Manage root access
Usage: magisk-cli -r (options) (app)

Options:

    -a (uid) (app) - add app
    -x (uid) (app) - deny root access
    -e (uid) (app) - enable root access
    -d (uid) (app) - delete app
    -le - print list of rooted apps
```

2.Manage modules (install, enable, disable).
```
Manage modules
Usage: magisk-cli -m (options) (module)

Options:
    -i (zipfile) - install module
    -off (module) - disable module
    -u (module) - uninstall module
    -l - list modules
```

3.Manage new feature of Magisk - DenyList/Zygisk.
```
Manage DenyList
Usage: magisk-cli -z (options) (app)

Options:

    -e - activate DenyList
    -d - disable DenyList
    -a (app) - add app to DenyList
    -r (app) - delete app from DenyList
```

4.If you use old or custom build of Magisk, you can manage MagiskHide.
```
Manage MagiskHide
Usage: magisk-cli -H (options) (app)

    -e - activate MagiskHide
    -d - disable MagiskHide
    -a (app) - add app to MagiskHide
    -r (app) - delete app from MagiskHide
```
WARNING! You can add only app to MagiskHide, you can't manipulate his activities. I'll try to enable it in future releases

5.You can unpack, repack, patch boot.img. Magisk-CLI just calls `/data/adb/magisk/magiskboot` and `/data/adb/magisk/busybox`, there is no built-in functions for unpacking/repacking and hexpatching boot images, ramdisks, kernels.
```
Manage boot.img
Usage: magisk-cli -b (options) (/path/to/boot.img)
Usage: magisk-cli -b -r (/path/to/ramdisk) (options)

Options:

    -p - Patch stock boot.img
    -u - Unpack boot.img to kernel, dtb, ramdisk.cpio
    -uh - Unpack boot.img to kernel, dtb, ramdisk.cpio, header
    -r (ramdisk.cpio) (commands) - Ramdisk commands
    -hx (pattern1) (pattern2) (kernel/zImage) - hexpatch kernel
    -b (oldboot.img) (newboot.img) - rebuild boot.img. oldboot.img need for checking signatures, headers, cmdline and offsets for correct building of newboot.img. Youneed these files: ramdisk.cpio, oldboot.img, kernel, other files if they extracted (usually its 'extra', 'kernel_dtb', 'header')

Ramdisk commands:
    -x - unpack ramdisk.cpio
    -xg - unpack ramdisk.cpio.gz
    -xx - unpack ramdisk.cpio.xz
    -d (file) - delete file from ramdisk.cpio
    -a (mode) (file) (entry) - add file to ramdisk.cpio
    -m (mode) (directory) - create directory into ramdisk.cpio
    -o (directory) - unpack ramdisk to directory
```

## Module installing

Download magisk-cli.zip and install it from Magisk or Magisk's module manager. After installing, **WITHOUT REBOOTING**, go to the terminal and run:
```
 ~ $ su
 ~ # cd /data/adb/modules
 /data/adb/modules # rm -rf magisk-cli/upgrade
 /data/adb/modules # rm -rf ../modules_upgrade/magisk-cli
```
