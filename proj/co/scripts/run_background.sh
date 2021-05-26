#!/bin/bash

var=`ps aux | grep "python3 run.py"`
p_cnt=`ps aux | grep "python3 run.py" | wc -l`

if [ $p_cnt == 1 ]
then
	nohup python3 run.py >/dev/null 2>&1 &
	var=`ps aux | grep "python3 run.py"`
	pid=`echo $var | awk -e '$0 ~/^slawek / {print $2}'`
	echo "Run with PID $pid"
else
	pid=`echo $var | awk -e '$0 ~/^slawek / {print $2}'`
	echo "Already run with PID $pid"
fi
