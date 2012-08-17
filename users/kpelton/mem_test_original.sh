#!/bin/bash


MOD_LOC="/root/Loki/code/e1000.ko"
#set IFS to split by newline
IFS='
'
#command to get all interfaces




while [ 1 ] 
do
	lsmod | grep e1000
	if [ $? -eq 0 ]; then 
		rmmod e1000
	fi
	modprobe e1000
	ETH=( `ifconfig -a | egrep "eth[[:digit:]][[:digit:]]?" |sed s/:.*$//` )
	for i in "${ETH[@]}"
	do
		#let debugfs allocate some memory
		ifconfig $i up
	done
done

