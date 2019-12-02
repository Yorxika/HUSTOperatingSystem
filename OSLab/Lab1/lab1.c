#include <unistd.h>  //exex、fork头文件
#include <signal.h>  //signal头文件
#include<sys/types.h>  //wait头文件
#include<sys/wait.h>  //wait头文件
#include <stdlib.h>  //exit头文件
#include <stdio.h> 

int pipefd[2];   /*创建无名管道,pipefd[0]只能用于读;  pipe[1]只能用于写*/
int child_1, child_2;  //用于存储创建的子进程pid

void kill_child1() { //父进程杀死子进程1
	printf("Child process 1 is killed by parent!\n");
	close(pipefd[1]);  //关闭管道写端
	exit(0);  //子进程1退出
}

void kill_child2() { //父进程杀死子进程2
	printf("Child process 2 is killed by parent!\n");
	close(pipefd[0]);  //关闭管道读端
	exit(0);  //子进程2退出
}

void kill_childprocess() {  //父进程捕捉到中断信号，用系统调用Kill()向两个子进 程发出信号
	if (kill(child_1, SIGUSR1) < 0)
		printf("Kill child1 process error!\n");
	if (kill(child_2, SIGUSR1) < 0)
		printf("Kill child2 process error!\n");
}

int main(int argc, char const* argv[]) {

	int rtn; //函数执行返回值

	//创建管道 
	if (pipe(pipefd) == -1) {
		printf("Create a nameless pipe error!\n");
	}

	//创建第一个子进程
	switch (child_1 = fork())
	{
	case - 1:
		printf("Create child process failed!\n");
		break;
	case 0:
		close(pipefd[0]);  //子进程1关闭读端
		signal(SIGINT, SIG_IGN);  //SIGINT:由Interrupt Key产生，通常是CTRL+C或者DELETE。
													 //发送给所有ForeGround Group的进程
		signal(SIGUSR1, kill_child1);   // 设置进程处理信号

		int i = 1;
		while (1) { //死循环里每秒通过管道发送信号给子进程2
			sleep(1);
			write(pipefd[1], (void*)&i, sizeof(int));
			i++;
		}
		break;
	default:
		break;
	}

	//创建第二个子进程
	switch (child_2 = fork())
	{
	case -1:
		printf("Create child process failed!\n");
		break;
	case 0:
		close(pipefd[1]);  //子进程关闭写端
		signal(SIGINT, SIG_IGN);  //SIGINT:由Interrupt Key产生，通常是CTRL+C或者DELETE。
											 //发送给所有ForeGround Group的进程
		signal(SIGUSR1, kill_child2);   // 设置进程处理信号

		int i = 1;
		while (1) { //死循环里每秒通过管道发送信号给子进程2
			read(pipefd[0], (void*)&i, sizeof(int));
			printf("I have received your message %d times!\n", i);
			i++;
		}
		break;
	default:
		break;
	}

	signal(SIGINT,kill_childprocess);  //当捕捉到中断信号后
	//父进程用系统调用Kill()向两个子进 程发出信号

	//等到两个子进程结束
	waitpid(child_1, &rtn, 0);
	waitpid(child_2, &rtn, 0);

	// 关闭管道
	close(pipefd[0]);
	close(pipefd[1]);

	printf("Parent Process is killed!\n");
	return 0;
}
