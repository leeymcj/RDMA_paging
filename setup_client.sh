#!/bin/sh
rm server_list.txt
for n in 1 2 3 4 5 
do
port=$(ssh cp-$n cat RDMA.port)
echo "cp-$n $port" >> server_list.txt
done
