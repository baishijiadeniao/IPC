#include<stdio.h>
#include<error.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/time.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>

double gettime(struct timeval* begin,struct timeval* end){
    return (end->tv_sec - begin->tv_sec+end->tv_usec* 1.0 /1000000-begin->tv_usec* 1.0 /1000000);
}

int main(int argc,char* argv[]){
    int sum,count,size,yes;
    struct timeval begin,end;
    char* buff;
    double t;
    int fd,tmp;
    struct sockaddr_in address;
    if(argc !=3){
        perror("usage instruction: ./application <size> <count>");
    }
    size=atoi(argv[1]);
    count=atoi(argv[2]);
    buff=static_cast<char*>(malloc(size));
    memset(&address,0,sizeof(address));
    if(fork()==0){
        //使用SOCK_DGRAM套接字
        fd=socket(AF_INET,SOCK_DGRAM,0);
        yes=1;
        setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
        address.sin_family=AF_INET;
        address.sin_addr.s_addr=INADDR_ANY;
        address.sin_port=htons(6789);
        tmp=bind(fd,(struct sockaddr*)&address,sizeof(address));
        if(tmp<0){
            perror("bind error");
            return 1;
        }  
        //SOCK_DGRAM套接字数据的发送和接收是同步的，接收次数应该和发送次数相同。
        for(int i=0;i<count;i++)
        {
            tmp=recv(fd,buff,size,0);
            if(tmp==0){
                break;
            }
            if(tmp ==-1){
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
        fd=socket(AF_INET,SOCK_DGRAM,0);
        address.sin_family=AF_INET;
        address.sin_addr.s_addr=inet_addr("127.0.0.1");
        address.sin_port=htons(6789);
        tmp=connect(fd,(struct sockaddr*)&address,sizeof(address));
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
                return 1;
            }
            sum +=tmp;
        }
        gettimeofday(&end,NULL);
        t=gettime(&begin,&end);
        printf("%.0fMB/s %.0fmsg/s\n",
            count * size * 1.0 / (t * 1024 * 1024),
            count * 1.0 / t);
    }
    return 0;
}