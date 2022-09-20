#include<stdio.h>
#include<error.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/time.h>
#include<sys/socket.h>
#include<sys/un.h>
#include <cstddef>
#include<string.h>
#include<arpa/inet.h>

double gettime(struct timeval* begin,struct timeval* end){
    return (end->tv_sec + end->tv_usec * 1.0 / 1000000) -(begin->tv_sec + begin->tv_usec * 1.0 / 1000000);
}

int main(int argc,char* argv[]){
    int sum,count,size,yes,len;
    struct timeval begin,end;
    char* buff;
    double t;
    int fd,tmp;
    struct sockaddr_un address;
    if(argc !=3){
        perror("usage instruction: ./application <size> <count>");
    }
    size=atoi(argv[1]);
    count=atoi(argv[2]);
    buff=static_cast<char*>(malloc(size));
    memset(&address,0,sizeof(address));
    if(fork()==0){
        fd=socket(AF_UNIX,SOCK_STREAM,0);
        unlink("./uds_IPC");
        strcpy(address.sun_path,"./uds_IPC");
        address.sun_family=AF_UNIX;
        //offsetof计算sun_path在结构体中的偏移量，其实在bind中直接用sockaddr_un的大小也可以
        len= offsetof(struct sockaddr_un,sun_path)+strlen("./uds_IPC");
        tmp=bind(fd,(struct sockaddr*)&address,len);
        if(tmp<0){
            perror("bind error");
            return 1;
        }
        tmp=listen(fd,5);
        if(tmp<0){
            perror("listen error");
            return 1;
        }
        int connfd=accept(fd,NULL,NULL);
        if(connfd<0){
            perror("accept error");
            return 1;
        }        
        for(;;)                                 //可能产生孤儿进程，所以要跳出进程
        {
            tmp=read(connfd,buff,size);
            if(tmp==0){
                break;
            }
            else if(tmp ==-1){
                perror("read error");
                return 1;
            }
            sum +=tmp;
        }
        if(sum != size*count){
            printf("sum error: sum:%d != size*count:%d",sum,size*count);
            return 1;
        }
        close(fd);
    }else{
        sleep(1);
        fd=socket(AF_UNIX,SOCK_STREAM,0);
        address.sun_family=AF_UNIX;
        strcpy(address.sun_path,"./uds_IPC");
        len= offsetof(struct sockaddr_un,sun_path)+strlen("./uds_IPC");
        tmp=connect(fd,(struct sockaddr*)&address,len);
            if(tmp<0){
                perror("connect error");
                return 1;
            }
        gettimeofday(&begin,NULL);
        for(int i=0;i<count;i++)
        {
            tmp=write(fd,buff,size);
            if(tmp != size){
                perror("write error");
                return -1;
            }
            sum +=tmp;
        }
        gettimeofday(&end,NULL);
        t=gettime(&begin,&end);
        close(fd);
        //这个东西要放里面不能放外面，不然子进程和父进程都会输出数据导致数据重复
        printf("%.0fMB/s %.0fmsg/s\n",
            count * size * 1.0 / (t * 1024 * 1024),
            count * 1.0 / t);
    }
    return 0;
}