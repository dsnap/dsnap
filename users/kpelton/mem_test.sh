#!/bin/bash


MOD_LOC="/root/Loki/code/e1000.ko"
#set IFS to split by newline
IFS='
'
#command to get all interfaces
ETH=( `ifconfig -a | egrep "eth[[:digit:]][[:digit:]]?" |sed s/:.*$//` )




while [ 1 ] 
do
	lsmod | grep e1000
	if [ $? -ne 0 ]; then 
		insmod $MOD_LOC
	fi
	rmmod e1000
	insmod e1000.ko
	for i in "${ETH[@]}"
	do
		ifconfig $i up
	done
	sleep 1	
done

