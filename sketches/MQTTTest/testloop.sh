#!/bin/bash

while true ; do
	mosquitto_pub -h nodex.local -u demo -P demo -t "esp00/Power/set" -m 1
	sleep 1
	mosquitto_pub -h nodex.local -u demo -P demo -t "esp00/Power/set" -m 0
	sleep 1
done
