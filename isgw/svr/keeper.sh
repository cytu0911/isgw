#!/bin/bash
###############################################################################
# function:���ָ���Ľ���
# trait	  :
# author:away
# history: 
# init 2005-09-02
#      2006-07-26 ͬʱ���Լ�ض������
#      2008-05-23 ����������в������������в���Ϊ׼
# 
###############################################################################
#��ؽ�����
KEEP_COUNT=2 

#��һ�������Ϣ������������ģ�������д
#������������·������ص�ʱ����
PROCESS_NAME[0]="isgw_svrd"
START_PAHT[0]="/usr/local/isgw/bin/start.sh isgw_svrd"
INTERVAL[0]="10"
LOG_PATH[0]="/usr/local/isgw/bin/keeper.log"

PROCESS_NAME[1]="isgw_svrd"
START_PAHT[1]="/usr/local/isgw/bin/start.sh isgw_svrd"
INTERVAL[1]="10"
LOG_PATH[1]="/usr/local/isgw/bin/keeper.log"

# �����в���ֻ֧�ּ��һ������
if [ $# -gt 0 ]
then
{
    KEEP_COUNT=1
    PROCESS_NAME[0]="$1"
    START_PAHT[0]="$2"
    INTERVAL[0]="$3"
    LOG_PATH[0]="$4"
}
fi

###############################################################################
# desc		:
# input		:
# output	:
###############################################################################
main()
{
    typeset i=0
    while [ $i -lt ${KEEP_COUNT} ]
    do    
	    keep_proc "${PROCESS_NAME[$i]}" "${START_PAHT[$i]}" "${INTERVAL[$i]}" "${LOG_PATH[$i]}"&
	    i=`expr $i + 1`
    done

    return $?
}

###############################################################################
. /usr/local/shcom/tools.inc

cd ${0%/*}

#echo `pwd`

main "$@"
exit $?
