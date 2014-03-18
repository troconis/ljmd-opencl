#!/bin/bash

#utility to bench  the time run vs the number of spawned threads

device=$1
threads=$2
infile=$3
benchfile=$4
echo "device $device threads $threads infile $infile benchfile $benchfile"

rm $benchfile
for t in $threads 
do
    #echo "$t: device $device threads $threads infile $infile benchfile $benchfile"
    /usr/bin/time  --output=$benchfile --append --format="%C %e seconds %K kilobytes" ./ljmd-cl $device $t < $infile 
done

