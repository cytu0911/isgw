#!/bin/bash

uin=29320360
while true
do
    ./client 172.25.40.94 5693 "cmd=1&uin=$uin" 1000000 1 & 
    uin=`expr $uin + 1`
    sleep 5
done 

#grep "ISGWIntf putq msg to ISGWMgrSvc" isgw_svrd.log | awk -F',' '{ print $6}' | sort | uniq | 
