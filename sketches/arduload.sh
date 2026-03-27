#!/bin/bash
# arduload.sh set given preferences and load given app in the legacy IDE
# 2023-04-12

declare PREFDIR="$HOME/ARDUINO/Espressif/preferences"

if [ $# -ne 2 ] ; then
	echo "usage $(basename $0) board application_dir_path"
	echo "boards: "
	ls "$PREFDIR"
	echo "example $(basename $0) ESP32 Daikin/DaikinRemote"
	exit 1
fi

declare Board=$1
declare AppDirPath=$2
declare InoPath="$AppDirPath/$(basename $AppDirPath).ino"

if [ -f "$PREFDIR/$Board" ] ; then
	if ! cp "$PREFDIR/$Board" ~/.arduino15/preferences.txt ; then
		exit 1
	fi
else
	echo "invalid $Board"
	exit 1
fi

if [ -f "$InoPath" ] ; then
	declare ide_path="$HOME/arduino-ide" # on hpnb17
	if [ ! -f "$ide_path" ] ; then
		ide_path="$HOME/Documents/arduino/arduino" # on debian104
	fi
	"$ide_path" "$InoPath" >/dev/null 2>&1 &
else
	echo "application $InoPath not found"
	exit 1
fi