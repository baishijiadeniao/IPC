#include<stdio.h>
#include<error.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/time.h>
#include<sys/socket.h>

double gettime(struct timeval* begin,struct timeval* end){
    return (end->tv_sec - begin->tv_sec+end->tv_usec* 1.0 /1000000-begin->tv_usec* 1.0 /1000000);
}

int main(int argc,char* argv[]){
    int sockfd[2],sum,count,size;
    struct timeval begin,end;
    char* buff;
    double t;
    if(argc !=3){
        perror("usage instruction: ./application <size> <count>");
    }
    size=atoi(argv[1]);
    count=atoi(argv[2]);
    buff=static_cast<char*>(malloc(size));
    int tmp=socketpair(AF_UNIX,SOCK_STREAM,0,sockfd);
    if(tmp<0){
        perror("create socketpair error");
        return 1;
    }
    if(fork()==0){
        for(int i=0;i<count;i++)
        {
            tmp=read(sockfd[0],buff,size);
            if(tmp<0){
                perror("read error");
                return 1;
            }
            sum +=tmp;
        }
    }else{
        gettimeofday(&begin,NULL);
        for(int i=0;i<count;i++)
        {
            tmp=write(sockfd[1],buff,size);
            if(tmp != size){
                perror("write error");
                return 1;
            }
            sum +=tmp;
        }
        gettimeofday(&end,NULL);
        t=gettime(&begin,&end);
        printf("%.0fMB/s %.0fmsg/s\n",count * size * 1.0 / (t * 1024 * 1024),count * 1.0 / t);
    }
    return 0;
}