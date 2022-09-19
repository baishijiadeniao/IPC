#include<stdio.h>
#include<error.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/time.h>
#include<sys/socket.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<string.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define write_sem 0
#define read_sem 1

double gettime(struct timeval* begin,struct timeval* end){
    return (end->tv_sec - begin->tv_sec+end->tv_usec* 1.0 /1000000-begin->tv_usec* 1.0 /1000000);
}

union semun{
    int val;
    struct semid_id *buf;       //信号集
    unsigned short *array;
};

//设置上信号集中的信号量数量,sem_id是信号集id，semnum指定某一信号量，init_value是要初始化的信号量值
void sem_init(int sem_id,int sem_num,int init_value){
    union semun sem_union;
    sem_union.val=init_value;
    if(semctl(sem_id,sem_num,SETVAL,sem_union)){
        perror("semctl fail");
        exit(-1);
    }
}

//V操作，信号量+1
void sem_release(int sem_id,int sem_num){
    struct sembuf semb;
    semb.sem_num=sem_num;
    semb.sem_op=1;
    semb.sem_flg=0;
    if(semop(sem_id,&semb,1)==-1){
        perror("semop fail");
        exit(-1);
    }
}

//P操作，信号量-1
void sem_reserve(int sem_id,int sem_num){
    struct sembuf semb;
    semb.sem_num=sem_num;
    semb.sem_op=-1;
    semb.sem_flg=0;
    if(semop(sem_id,&semb,1)==-1){
        perror("semop fail");
        exit(-1);
    }
}

int main(int argc,char* argv[]){
    int count,size,sem_id;
    int shm_id;
    struct timeval begin,end;
    char* buff;
    double t;
    if(argc !=3){
        perror("usage instruction: ./application <size> <count>");
    }
    size=atoi(argv[1]);
    count=atoi(argv[2]);
    buff=static_cast<char*>(malloc(size));

    if(fork()==0){
        //创建信号集
        sem_id=semget(SEM_KEY,2,0600 | IPC_CREAT);
        if(sem_id==-1){
            perror("semget error");
            return 1;
        }
        sem_init(sem_id,write_sem,0);
        sem_init(sem_id,read_sem,1);
        //创建共享内存
        shm_id=shmget(SHM_KEY,size,0600 | IPC_CREAT);
        if(shm_id ==-1){
            perror("shmget error");
            return 1;
        }
        //创建指向共享存储器的指针
        void *addr=shmat(shm_id,NULL,0);
        if(addr==(void*)-1){
            perror("shmat error");
            return 1;
        }
        for(int i=0;i<count;i++)
        {
            //写操作
            sem_reserve(sem_id,read_sem);
            memcpy(buff,addr,size);
            sem_release(sem_id,write_sem);
        }
        //进程脱离共享存储器
        if((shmdt(addr))==-1){
            perror("shmdt error");
            return 1;
        }
    }else{
        sleep(1);
        sem_id=semget(SEM_KEY,0,0);
        if(sem_id==-1){
            perror("semget error");
            return 1;
        }
        shm_id=shmget(SHM_KEY,0,0);
        if(shm_id ==-1){
            perror("shmget error");
            return 1;
        }
        void *addr=shmat(shm_id,NULL,0);
        if(addr==(void*)-1){
            perror("shmat error");
            return 1;
        }
        gettimeofday(&begin,NULL);
        for(int i=0;i<count;i++)
        {
            sem_reserve(sem_id,write_sem);
            //读操作
            memcpy(addr,buff,size);
            sem_release(sem_id,read_sem);
        }
        gettimeofday(&end,NULL);
        t=gettime(&begin,&end);
        //删除信号量集和，由于两个信号量是在同一集合所以只用删除一次
        union semun dummy;
        semctl(sem_id,write_sem,IPC_RMID,dummy);
        //进程脱离共享存储器
        if((shmdt(addr))==-1){
            perror("shmdt error");
            return 1;
        }
        //释放共享内存资源
        shmctl(shm_id,IPC_RMID,0);
        printf("%.0fMB/s %.0fmsg/s\n",count * size * 1.0 / (t * 1024 * 1024),count * 1.0 / t);
    }
    return 0;
}