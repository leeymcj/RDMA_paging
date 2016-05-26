#!/bin/sh
#page.sh node_num size
host="cp-$1"
#evoke remote server
ssh $host rdma-server $2&
pid=$!
#wait until port available
sleep 0.001
#get poprt
port=$(ssh cp-$1 cat RDMA.port)

sudo ./page cp-$1 $port $2

kill $pid
