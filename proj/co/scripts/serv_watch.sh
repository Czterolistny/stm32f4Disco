#!/bin/sh

html_hdr=`echo "DOCTYPE_html"`
while :
do
	l=$(curl http://192.168.1.105:8500/param.html 2>/dev/null | head -n1\
	       	| tr -d "<>!" | tr " " "_")
	if [ "$l" != "$html_hdr" ]
	then
		echo $(date +%Y-%m-%d-%T)
		./restart.sh
	fi
	sleep 30
done
