#!/bin/bash


MOD_LOC="/root/Loki/code/e1000.ko"
#set IFS to split by newline
IFS='
'
INTERVAL=0.001
#command to get all interfaces

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
		#ping previous interface
		if [ $j -gt 0 ]; then
			prev=$[j-1]	
			ping -I $i 192.168.0.1 -f  >/dev/null &
		fi
	done
	sleep 5

