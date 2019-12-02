#include <unistd.h>  //exex��forkͷ�ļ�
#include <signal.h>  //signalͷ�ļ�
#include<sys/types.h>  //waitͷ�ļ�
#include<sys/wait.h>  //waitͷ�ļ�
#include <stdlib.h>  //exitͷ�ļ�
#include <stdio.h> 

int pipefd[2];   /*���������ܵ�,pipefd[0]ֻ�����ڶ�;  pipe[1]ֻ������д*/
int child_1, child_2;  //���ڴ洢�������ӽ���pid

void kill_child1() { //������ɱ���ӽ���1
	printf("Child process 1 is killed by parent!\n");
	close(pipefd[1]);  //�رչܵ�д��
	exit(0);  //�ӽ���1�˳�
}

void kill_child2() { //������ɱ���ӽ���2
	printf("Child process 2 is killed by parent!\n");
	close(pipefd[0]);  //�رչܵ�����
	exit(0);  //�ӽ���2�˳�
}

void kill_childprocess() {  //�����̲�׽���ж��źţ���ϵͳ����Kill()�������ӽ� �̷����ź�
	if (kill(child_1, SIGUSR1) < 0)
		printf("Kill child1 process error!\n");
	if (kill(child_2, SIGUSR1) < 0)
		printf("Kill child2 process error!\n");
}

int main(int argc, char const* argv[]) {

	int rtn; //����ִ�з���ֵ

	//�����ܵ� 
	if (pipe(pipefd) == -1) {
		printf("Create a nameless pipe error!\n");
	}

	//������һ���ӽ���
	switch (child_1 = fork())
	{
	case - 1:
		printf("Create child process failed!\n");
		break;
	case 0:
		close(pipefd[0]);  //�ӽ���1�رն���
		signal(SIGINT, SIG_IGN);  //SIGINT:��Interrupt Key������ͨ����CTRL+C����DELETE��
													 //���͸�����ForeGround Group�Ľ���
		signal(SIGUSR1, kill_child1);   // ���ý��̴����ź�

		int i = 1;
		while (1) { //��ѭ����ÿ��ͨ���ܵ������źŸ��ӽ���2
			sleep(1);
			write(pipefd[1], (void*)&i, sizeof(int));
			i++;
		}
		break;
	default:
		break;
	}

	//�����ڶ����ӽ���
	switch (child_2 = fork())
	{
	case -1:
		printf("Create child process failed!\n");
		break;
	case 0:
		close(pipefd[1]);  //�ӽ��̹ر�д��
		signal(SIGINT, SIG_IGN);  //SIGINT:��Interrupt Key������ͨ����CTRL+C����DELETE��
											 //���͸�����ForeGround Group�Ľ���
		signal(SIGUSR1, kill_child2);   // ���ý��̴����ź�

		int i = 1;
		while (1) { //��ѭ����ÿ��ͨ���ܵ������źŸ��ӽ���2
			read(pipefd[0], (void*)&i, sizeof(int));
			printf("I have received your message %d times!\n", i);
			i++;
		}
		break;
	default:
		break;
	}

	signal(SIGINT,kill_childprocess);  //����׽���ж��źź�
	//��������ϵͳ����Kill()�������ӽ� �̷����ź�

	//�ȵ������ӽ��̽���
	waitpid(child_1, &rtn, 0);
	waitpid(child_2, &rtn, 0);

	// �رչܵ�
	close(pipefd[0]);
	close(pipefd[1]);

	printf("Parent Process is killed!\n");
	return 0;
}
