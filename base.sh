#!/bin/sh
#page.sh node_num size
host="cp-$1"
#evoke remote server
ssh $host rdma-server $2&
#wait until port available
sleep 0.001
#get poprt
port=$(ssh cp-$1 cat RDMA.port)

sudo ./base cp-$1 $port $2
