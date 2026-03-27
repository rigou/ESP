#!/bin/bash
# bwacli.sh - A script to compile and upload Arduino projects for ESP32
# 2025-06-09

# Parse arguments
if [[ $# -lt 1 ]] || [[ "$1" == '-h' ]] || [[ "$1" == '--help' ]]; then
	echo "Usage: $(basename $0) PROJ_PATH [[-u USB_DEVICE | -U USB_DEVICE] -o OTA_TARGET] [-f CPU_FREQ]"
	echo "USB_DEVICE must be a valid USB device name in /dev (e.g., ttyUSB0)."
	echo "OTA_TARGET must be a valid IP address or hostname."	
	echo "CPU_FREQ must be one of [40, 80, 160, 240]. Default is 80."
    echo "option -u to upload program to given usb device after compilation"
    echo "option -U to skip compilation and upload latest compiled program to given usb device"
    echo "option -o to upload program to given device over the air after compilation"
	echo "example: $(basename $0) Remote -u ttyUSB0 # compile and upload to USB device"
    echo "example: $(basename $0) Remote -U ttyUSB0 # direct upload to USB device without compilation"
	echo "example: $(basename $0) Remote -o 192.168.2.30 -f 240 # compile and upload to OTA target with CPU frequency 240 MHz"
	exit 1
fi

declare PROJ_PATH="$1"
shift

declare USB_DEVICE=""
declare OTA_TARGET=""
declare -i CPU_FREQ=80
declare -i COMPILE=1 # compile by default except if option -U is given (direct upload)

while getopts ":u:U:o:f:" opt; do
    case $opt in
        u)
            if [[ -n "$OTA_TARGET" ]]; then
                echo "Error: Only one of -u or -o may be specified."
                exit 1
            fi
            if [[ ! -c "/dev/$OPTARG" ]]; then
                echo "Error: USB device '/dev/$OPTARG' does not exist or is not a character device."
                exit 1
            fi
            USB_DEVICE="$OPTARG"
            ;;
        U)  if [[ -n "$OTA_TARGET" ]]; then
                echo "Error: Only one of -u or -o may be specified."
                exit 1
            fi
            if [[ ! -c "/dev/$OPTARG" ]]; then
                echo "Error: USB device '/dev/$OPTARG' does not exist or is not a character device."
                exit 1
            fi
            USB_DEVICE="$OPTARG"
            COMPILE=0 # skip compilation if -U is used
            ;;
        o)
            if [[ -n "$USB_DEVICE" ]]; then
                echo "Error: Only one of -u or -o may be specified."
                exit 1
            fi
            OTA_TARGET="$OPTARG"
            ;;
        f)
            if [[ ! "$OPTARG" =~ ^(40|80|160|240)$ ]]; then
                echo "Error: Invalid CPU_FREQ value '$OPTARG'. Allowed values are 40, 80, 160, 240."
                exit 1
            fi
            CPU_FREQ="$OPTARG"
            ;;
        \?)
            echo "Invalid option: -$OPTARG"
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument."
            exit 1
            ;;
    esac
done

if [[ ! -d "$PROJ_PATH" ]]; then
	echo "Error: '$PROJ_PATH' is not a directory or does not exist."
	exit 1
fi

declare -r APPNAME=$(basename "$PROJ_PATH")
declare -r DIRBUILD=/tmp/$APPNAME/build

# compile --verbose to troubleshoot
# compile --build-property build.extra_flags=-DPIN=2		# define macro PIN=2 for the preprocessor
# compile --build-property "build.extra_flags=-DPIN=2 \"-DMY_DEFINE=\"hello world\"\""	# define 2 macros, quoting spaces in the 2nd one
# compile --build-property compiler.cpp.extra_flags=-Wextra	# define additional option -Wextra for the compiler
if [[ $COMPILE -eq 1 ]]; then
    declare -i COMPILE_STATUS=0
    echo "arduino-cli compile --warnings all --fqbn esp32:esp32:esp32  --board-options "CPUFreq=$CPU_FREQ" --build-path "$DIRBUILD" "$PROJ_PATH""
    arduino-cli compile --warnings all --fqbn esp32:esp32:esp32 --board-options "CPUFreq=$CPU_FREQ" --build-path "$DIRBUILD" "$PROJ_PATH" |tee "/tmp/${APPNAME}_compile_$(date '+%Y%m%dT%H%M%S').log"
    COMPILE_STATUS=${PIPESTATUS[0]}
    if [[ $COMPILE_STATUS -ne 0 ]]; then
        echo "Error: Compilation failed with status $COMPILE_STATUS."
        exit $COMPILE_STATUS
    fi
    ls -l "$DIRBUILD/$APPNAME.ino.bin"
fi

if [[ -n "$USB_DEVICE" ]]; then
	# Upload via USB
	echo "Uploading $APPNAME to USB device: $USB_DEVICE"
	arduino-cli upload -p "/dev/$USB_DEVICE" --fqbn esp32:esp32:esp32 --input-dir "$DIRBUILD" "$PROJ_PATH"
elif [[ -n "$OTA_TARGET" ]]; then
	# Upload via OTA
	echo "Uploading $APPNAME to OTA target: $OTA_TARGET"
	"$(dirname $0)"/OTAupload.sh "$APPNAME" "$OTA_TARGET"
fi
