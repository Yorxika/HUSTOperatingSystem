//#include <linux/sem.h>
#include <sys/sem.h>
#include <stdio.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

//#include "lab3.h"

#define MAXSIZE 1024  //环形缓冲区大小
#define LEN 10  //环形缓冲区长度
#define true 1

union semun
{
 int val; //信号量初始值                   
 struct semid_ds *buf;        
 unsigned short int *array;  
 struct seminfo *__buf;      
}; 

int child_1, child_2;  //用于存储创建的子进程pid
int semid = 0;  //信号灯集
char (*buf) [MAXSIZE];  
int* pointer;  //用于标明读、写、最后一次写数目的指针

void P(int semid,int index)
{
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = -1;
	sem.sem_flg = 0;   //操作标记：0或IPC_NOWAIT等
	semop(semid, &sem, 1);  //1表示命令执行个数
	return;
}

void V(int semid,int index)
{
	struct  sembuf sem;
	sem.sem_num = index;
	sem.sem_op = 1;
	sem.sem_flg = 0;
	semop(semid, &sem, 1);
	return;
}

int main()
{
	int rtn;

    //创建键值为0，权限值为0666的信号集，信号灯个数为2并返回信号量集的IPC标识符
	if ((semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666)) < 0)
	{
		printf("Create Semaphore Error ! Error code: %d.\n",semid);
	}

	//创建一个共享内存对象并返回共享内存标识符,该对象为长度为10的size为MAXSIZE的字符数组
	int shm;
	if ((shm = shmget(IPC_PRIVATE, sizeof(char[MAXSIZE])*LEN,IPC_CREAT | 0666)) < 0)
	{
		printf("Create Shared Memory Segment Error ! Error code: %d.\n", shm);
	}

	//创建3个共享内存的指针,分别用于读 写 已经读入个数
	int index;
	if ((index = shmget(IPC_PRIVATE, sizeof(int)*3,IPC_CREAT | 0666)) < 0)
	{
		printf("Create Shared Memory Segment Error ! Error code: %d.\n", index);
	}

	//连接共享内存标识符为shmid的共享内存，连接成功后把共享内存区对象映射到调用进程的地址空间
	//读写模式
	buf = shmat(shm,NULL, SHM_R | SHM_W);
	pointer = shmat(index, NULL, SHM_R | SHM_W);
	memset(pointer,0,sizeof(int)*3);
    
	//初始化sem.h里的semun联合,设置信号量的值
	//index = 0的信号灯为NUM，index = 1 的信号灯为0
	union semun sem_union;
	sem_union.val = LEN;
	semctl(semid, 0, SETVAL, sem_union);  
	sem_union.val = 0;
	semctl(semid, 1, SETVAL, sem_union);

	//创建第一个子进程用于读
	switch (child_1 = fork())
	{
	case -1 :
		printf("Create child process failed!\n");
		break;
	case 0:
	{
		FILE* fp = fopen("input.txt", "r");
		int haveRead = 0;
		while (true) {
			P(semid, 0);
			haveRead = fread(buf[pointer[0]], sizeof(char), MAXSIZE, fp);
			printf("haveRead is : %d\n", haveRead);
			pointer[0] = (pointer[0] + 1) % LEN;
			if (feof(fp)) {  //说明读完了
				printf("Finish read!\n");
				pointer[2] = haveRead;
				V(semid, 1);  //哪怕是读完了也要释放最后一个信号灯
				break;
			}
			V(semid, 1);
		}
		exit(0);
		//break;
	}
	default:
		break;
	}

	//创建第二个字进程用于写
	switch (child_2 = fork())
	{
	case -1:
		printf("Create child process failed!\n");
		break;
	case 0:
	{
		FILE* fw = fopen("output.txt", "w");
		int haveWrite = 0;
		while (true) {
			P(semid, 1);
			if ( (pointer[1] + 1) % LEN == pointer[0]) { //最后一个缓冲区
				if (pointer[2] != 0) {  //要先判断是最后一个缓冲区之后才可以判断是否读完，否则会导致提前终止写入
					haveWrite += fwrite(buf[pointer[1]], sizeof(char), pointer[2], fw);
					printf("haveWrite is : %d\n", haveWrite);
					printf("Finish write!\n");
					break;
				}
				else haveWrite += fwrite(buf[pointer[1]], sizeof(char), MAXSIZE, fw);
				pointer[1] = (pointer[1] + 1) % LEN;
			}
			else
			{
				haveWrite += fwrite(buf[pointer[1]], sizeof(char), MAXSIZE, fw);
				pointer[1] = (pointer[1] + 1) % LEN;
			}
			printf("haveWrite is : %d\n", haveWrite);
			V(semid, 0);
		}
		exit(0);
		//break;
	}
	default:
		break;
	}

	//等到两个子进程结束
	waitpid(child_1, &rtn, 0);
	waitpid(child_2, &rtn, 0);

	//删除信号灯集
	semctl(semid, 0, IPC_RMID,sem_union);

	//删除共享内存
	shmctl(shm, IPC_RMID, 0);
	shmctl(index, IPC_RMID, 0);

	return 0;
}
