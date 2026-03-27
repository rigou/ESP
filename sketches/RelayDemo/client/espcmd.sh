#!/bin/bash
# espcmd.sh - send commands to an Espressif device
# 2022-04-15

declare -c VERSION="v1.1"
declare -i EspPort=59000
declare -c FLOG="$HOME/log/espcmd.log"

function usage {
    echo "$(basename $0) $VERSION"
	echo "usage: $(basename $0) [-L] [-n] [-p port] devicename command"
	echo "option -L copies output to the log file at $FLOG"
    echo "option -n tells not to ping the device before sending command"
	echo "option -p specifies the listening port of the device, defaults to $EspPort"
	echo "example: $(basename $0) -L clim0 T20"
}

function ping_device {
    local -i retval=1
    local devicename=$1 # eg "clim1"
    local -i counter=0
    while [ $counter -lt 3 ] ; do
        sleep 1
        if ping -c 1 "$devicename" >/dev/null ; then
            retval=0
            break
        fi
        counter+=1
    done
    return $retval
}

function main {
	local -i write_log=0
    local -i ping_first=1
	# parse option and arguments
	if [ $# -ge 2 ] ; then
		while getopts Lnp: option
		do
			case $option in
				L)	write_log=1;;
                n)  ping_first=0;;
				p)	EspPort=$OPTARG;;
				*)	usage
					exit 1;;
			esac
		done
		shift $(($OPTIND -1)) 
	else
		usage
		exit 1
	fi
	if [ $# -ne 2 ] ; then
		usage
		exit 1
	fi
	
	if [ $write_log -eq 1 ] ; then
		exec >  >(tee -ia $FLOG)
		exec 2> >(tee -ia $FLOG >&2)
        echo "$(date '+%Y%m%dT%H%M%S') $*"
	fi
	
    local -i device_online=0
    if [ $ping_first -eq 1 ] ; then
        if ping_device "$1" ; then
            device_online=1
        else
            echo "$1 is offline"
        fi
    else
        device_online=1
    fi
    if [ $device_online -eq 1 ] ; then
        echo "$2" |nc -w 10 "$1" "$EspPort"
    fi
	return 0
}

main "$@"
exit $?
