if [ $# -lt 1 ]
then
{
    echo "Usage:$0 <svrd_name>"
    exit 1
}
fi

SVRD_NAME=$1
export ISGW_HOME=../
export ISGW_BIN=${ISGW_HOME}/bin/
export ISGW_CFG=${ISGW_HOME}/cfg/
export LD_LIBRARY_PATH=./

# for itc oper
export SYS_CONF_PATH=${ISGW_CFG}
export SYS_SOCKET_TIMEOUT=2

ulimit -c unlimited           #����core�ļ���������core�ļ���С
ulimit -n 20480               #�޸����Ӿ���� 
ulimit -s 2048                #�޸��̵߳Ķ�ջ��С�����ⵥ���߳���ʹ���ڴ�̫��
${ISGW_BIN}/${SVRD_NAME} 1>>${ISGW_BIN}/start.log 2>&1 &
