#include <linux/sem.h>
#include <stdio.h>
#include <pthread.h>

int a = 0;  //公共变量a
int semid = 0;
pthread_mutex_t mutex1, mutex2;

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

void* subp1(void *arg)
{
	for(int i = 0 ; i < 100 ; i++)
	{
		P(semid,1);
		//pthread_mutex_lock(&mutex2);
		printf("thread 1 : a now is %d.\n", a);
		V(semid,0);
		//pthread_mutex_unlock(&mutex1);
	}
	return NULL;
}

void* subp2(void *arg)
{
	for (int i = 1; i < 101; i++)
	{
		P(semid,0);
		//pthread_mutex_lock(&mutex1);
		a += i;
		printf("thread 2 : i is %d , a now is : %d. \n", i,a);
		V(semid,1);
		//pthread_mutex_unlock(&mutex2);
	}
	return NULL;
}

int main()
{
	//pthread_mutex_init(&mutex1, NULL);
	//pthread_mutex_init(&mutex2, NULL);
	pthread_t p1, p2;

    //创建键值为0，权限值为0666的信号集，信号灯个数为2并返回信号量集的IPC标识符
	if ((semid = semget(0, 2, IPC_CREAT | 0666)) < 0)
	{
		printf("Create Semaphore Error ! Error code: %d.\n",semid);
	}

	//初始化sem.h里的semun联合,设置信号量的值
	//index = 0的信号灯为1，index = 1 的信号灯为0，用于让线程按p2 、p1的顺序执行
	union semun sem_union;
	sem_union.val = 1;
	semctl(semid, 0, SETVAL, sem_union);  
	sem_union.val = 0;
	semctl(semid, 1, SETVAL, sem_union);

	//创建2个线程
	pthread_create(&p1, NULL, subp1, NULL);
	pthread_create(&p2, NULL, subp2, NULL);

	//等待线程运行结束
	pthread_join(p1, NULL);
	pthread_join(p2, NULL);

	//删除信号灯集
	semctl(semid, 0, IPC_RMID,sem_union);
	return 0;
}