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
	insmod $MOD_LOC	
	ETH=( `ifconfig -a | egrep "eth[[:digit:]][[:digit:]]?" |sed s/:.*$//` )
	j=1
	for i in "${ETH[@]}"
	do	
		#let debugfs allocate some memory
		ifconfig $i 192.168.0.$j
		j=$[j+1]
	done
done

