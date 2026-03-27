#!/bin/bash
# OTAupload.sh - Upload binary of given app to the ESP device having given FQDN
# 2022-09-01
# 2023-04-05 ESPOTA is now version-independent
# 2025-03-29 updated path of espota.py, checked multiple arduino builds in /tmp, find target ip from hostname
# 2025-05-29 updated for compatibility with arduino-cli

if [ $# -ne 2 ] ; then
    echo "$(basename $0)  uploads the precompiled binary of given app to the ESP device having given hostname or IP address"
    echo "usage: $(basename $0) app_name device_hostname_or_ip"
    echo "example: $(readlink -f $0) Blink 192.168.2.30"
    exit 1
fi

# specify if you have compiled APPNAME with the Arduino IDE or with arduino-cli
# possible values: arduino-ide, arduino-cli
declare -r COMPILED_WITH="arduino-cli" 

declare -r APPNAME=$1
declare BINPATH=""

# Locate the binary file that we'll upload to the device
if [ $COMPILED_WITH = "arduino-ide" ] ; then
    # the build directory is defined by the IDE
    if [ "$(ls -d /tmp/arduino_build*|wc -l)" == "0" ] ; then
        echo "error: arduino build not found in /tmp"
        echo "compile your program and run $(basename $0) again"
        exit 2
    fi
    if [ "$(ls -d /tmp/arduino_build*|wc -l)" != "1" ] ; then
        echo "error: found multiple arduino builds in /tmp"
        echo '  - clean up the /tmp folder: rm -rf /tmp/arduino_build*'
        echo "  - recompile your program and run $(basename $0) again"
        ls -ld /tmp/arduino_build*
        exit 2
    fi
    BINPATH=$(ls /tmp/arduino_build*/$APPNAME.ino.bin)
else
    # the build directory is defined by option --build-path of the command arduino-cli compile
    declare -r DIRBUILD="/tmp/$APPNAME/build"
    BINPATH="$DIRBUILD/$APPNAME.ino.bin"
fi

if [ ! -f "$BINPATH" ] ; then
    echo "error: $BINPATH not found"
    exit 3
fi

declare ESPIP=$2
if ! [[ "$2" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]] ; then
    # resolve the hostname
    # query our master hosts file to find the IP corresponding to given hostname
    ESPIP=$(ssh nodex "grep $2 /etc/hosts |cut -f 1 -d ' '")
    if [ -z "$ESPIP" ] ; then
        echo "invalid hostname : $2 not found in //nodex/etc/hosts"
	exit 4
    fi
    echo "IP of $2 is $ESPIP"
fi

echo "Ready to upload $BINPATH"
stat $BINPATH |grep '^Modify'

declare $OTA_PASSWORD ; source $HOME/Projects/nogit/NetworkCredentials/NetworkCredentials.sh
declare ESPOTA=$HOME/.arduino15/packages/esp32/hardware/esp32/3.*/tools/espota.py
python $ESPOTA -i "$ESPIP" -p 3232 -P 18266 -a "$OTA_PASSWORD" -f "$BINPATH"
