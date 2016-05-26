while true;
do
ran=$(($RANDOM%5+1))
fmem=$(ssh cp-$ran free -m | grep "Mem:" | awk '{ print $4 }')
echo "cp-$ran has $fmem MB"
MB=$(($1 / 256))
echo "$1 pages is $MB MB"
if [ $fmem -gt $MB ]
then
./paging.sh $ran $1 >> rmm.data
fi
sleep 1 #interarrival time
done
