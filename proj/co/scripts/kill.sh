#!/bin/bash

var=`ps aux | grep "python3 run.py"`
p_cnt=`ps aux | grep "python3 run.py" | wc -l`

if [ $p_cnt == 2 ]
then
	pid=`echo $var | awk -e '$0 ~/^slawek / {print $2}'`
	sudo kill $pid
	echo "$pid killed"
	exit 1
else
	echo "Process isn't run"
	exit 0
fi
