#!/bin/sh
#measure proc/memory utilization
#and pagefault latency
#get free memory of random node
chmod +x page.sh
sudo insmod /usr/src/dune/kern/dune.ko

measure.sh&
while true;
do
ran=$(($RANDOM%2+1))
MB=$(($1 / 256))
echo "$1 pages is $MB MB"
./page.sh $ran $1 >> $(hostname).data
done

