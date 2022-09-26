#!/bin/bash

ipc_test="pipe fifo socketpair uds tcp shm udp"
ipc_size="128 256 512 1024 2048"
ipc_count=10000

#output function
write_log(){
    #局域变量，默认是全局变量
    local arg=""
    #如果参数有两个,$#表示参数个数
    if [ $# -eq 2 ]; then
        arg=$1
        shift
    fi
    echo $arg $1 >> ${log_file}
}

#check 文件是否存在
for test in ${ipc_test}
do
    #判断是否有此文件，-x file --用户可执行为真
    if [ ! -x ${test} ]; then
        echo "program ${test} doesn't  exit or is not a binary"
        exit 1
    fi
done

for test in ${ipc_test}
do
    log_file="./result.log"
    line1=""
    line2=""
    write_log "${test}"
    #表头:size, 将空格替换为'|'
    write_log "$(echo ${ipc_size} | tr [:space:] '|')"
    for data_size in ${ipc_size}
    do
        result=$(./${test} ${data_size} ${ipc_count})
        #按空格分割字符串，取出第一段
        #shell脚本不可以随便加空格
        line1="${line1}|$(echo $result | cut -d ' ' -f1)"
        line2="${line2}|$(echo $result | cut -d ' ' -f2)"
    done
    write_log "${line1}"
    write_log "${line2}"

    #修改格式，输出结果
    column -s "|"  -t ${log_file}
    echo ""

    #删除文件
    rm ${log_file}
    #如果中途退出程序会出现输出上一次的缓冲区的情况
done
    
