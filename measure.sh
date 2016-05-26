#!/bin/bash
#host=$(hostname) | cut -c 1-4
sudo iotop -o -b | grep Actual >> $(hostname).disk&
while true;
do
cpu=`top -b -n1 | grep "Cpu(s)" | awk '{print $2 + $3 + $4}'`
FREE_DATA=`free -m | grep Mem` 
CURRENT=`echo $FREE_DATA | cut -f3 -d' '`
TOTAL=`echo $FREE_DATA | cut -f2 -d' '`
mem=$(echo "scale = 2; $CURRENT/$TOTAL*100" | bc)
printf "%f %f\n" "$cpu" "$mem" >> $(hostname).mem&
sleep 1
done
