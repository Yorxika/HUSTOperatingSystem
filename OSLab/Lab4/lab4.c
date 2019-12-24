#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void printdir(char* dir, int deepth) {
	DIR* dp;
	struct dirent* entry;
	struct stat* statbuf;
	statbuf = (struct stat*)malloc(sizeof(struct stat));
	//printf("path = %s", dir);
	if ((dp = opendir(dir)) == NULL) {
		//��Ŀ¼���ɹ�
		printf("Open dir failed!\n");
		return;
	}

	printf("\n");
	for (int i = 0; i < deepth; i += 4)
		printf("\t");
	printf("%s\n\n", dir);  //��ӡ��ǰ·��

	chdir(dir);  //����Ŀ¼
		while ((entry = readdir(dp)) != NULL) {
			//����һ��Ŀ¼��
			lstat(entry->d_name,statbuf);  //�Ը�Ŀ¼�������Ϊ����,����lstat�õ���Ŀ¼��������Ϣ; 
			if (S_ISDIR(statbuf->st_mode)) {  //�����һ��Ŀ¼
				if ((!strcmp(entry->d_name, ".")) || (!strcmp(entry->d_name, "..")))
					continue;
				else {
					//����. ���� .. ��ӡĿ¼�����ȡ�Ŀ¼������Ϣ �ݹ����printdir,��ӡ��Ŀ¼����Ϣ,���е�depth+4; 
					//printf("Current dir  deepth is %d !", deepth);
					for (int i = 0; i < deepth; i += 4)
						printf("\t");
					printf("Depth: %d ", deepth);
					printInfo(*statbuf);
					printf("Dirname: %s\n", entry->d_name);
					printdir(entry->d_name, deepth + 4);
				}
			}
			else {
				//��ӡ�ļ�����ȡ��ļ�������Ϣ
				//printf("Current file deepth is %d !", deepth);
				for (int i = 0; i < deepth; i += 4)
					printf("\t");
				printf("Depth: %d ", deepth);
				printInfo(*statbuf);
				printf("Filename: %s\n", entry->d_name);
			}
	}
	chdir("../");
	closedir(dp);
}

void printFileDetails(mode_t mode){

	char str[11] = { 0 };

	str[0] = S_ISDIR(mode) ? 'd' : '-';   // Ŀ¼
	str[0] = S_ISCHR(mode) ? 'c' : '-';  //  �ַ��豸
	str[0] = S_ISBLK(mode) ? 'b' : '-';   // ���豸
	str[0] = S_ISLNK(mode) ?  'l' : '-';  //����
	str[0] = S_ISFIFO(mode) ? 'f' : '-';   //�ܵ��ļ�

	str[1] = mode & S_IRUSR ? 'r' : '-';   // ������
	str[2] = mode & S_IWUSR ? 'w' : '-';
	str[3] = mode & S_IXUSR ? 'x' : '-';

	str[4] = mode & S_IRGRP ? 'r' : '-';   // Ⱥ��
	str[5] = mode & S_IWGRP ? 'w' : '-'; 
	str[6] = mode & S_IXGRP ? 'x' : '-';

	str[7] = mode & S_IROTH ? 'r' : '-';   // ����
	str[8] = mode & S_IWOTH ? 'w' : '-';
	str[9] = mode & S_IXOTH ? 'x' : '-';

	printf("Mode: %s ", str);
}

void printInfo(struct stat state){

	printFileDetails(state.st_mode);

	//Ӳ���ӵ���Ŀ
	printf("%ld ", state.st_nlink);

	//�û���
	struct passwd* pwd;
	if ((pwd = getpwuid(state.st_uid)) != NULL) {
		printf("User: %s ", pwd->pw_name);
	}
	else {
		printf("%d ", state.st_uid);
	}

	//Ⱥ��
	struct group* grp;
	if ((grp = getgrgid(state.st_gid)) != NULL) {
		printf("Group: %s ", grp->gr_name);
	}
	else
		printf("%8d ", state.st_gid);


	//�ļ���С
	if(state.st_size < 1024)
		printf("Size: %lfB ", (double)state.st_size);
	else if (state.st_size < 1024 * 1024) 
		printf("Size: %lfK ", (double)state.st_size / 1024);
	else if (state.st_size < 1024 * 1024 * 1024) 
		printf("Size: %lfM ", (double)state.st_size / 1024 / 1024);
	else
		printf("Size: %lfG ", (double)state.st_size / 1024 / 1024 / 1024);

	//�޸�ʱ��
	time_t t = state.st_mtime;
	struct tm time;
	localtime_r(&t, &time);
	printf("Modified time:%d/", time.tm_year + 1900);
	printf("%d/", time.tm_mon + 1);
	printf("%d ", time.tm_mday);
	printf("%02d:%02d:%02d ", time.tm_hour, time.tm_min, time.tm_sec);
	//printf("%.12s ", ctime(state.st_mtime) + 4);

	//����ʱ��
	t = state.st_atime;
	localtime_r(&t, &time);
	printf("Access time:%d/", time.tm_year + 1900);
	printf("%d/", time.tm_mon + 1);
	printf("%d ", time.tm_mday);
	printf("%02d:%02d:%02d ", time.tm_hour, time.tm_min, time.tm_sec);

	//�޸���������
    /*t = state.st_ctime;
	localtime_r(&t, &time);
	printf("Chmod time:%d/", time.tm_year + 1900);
	printf("%d/", time.tm_mon + 1);
	printf("%d ", time.tm_mday);
	printf("%02d:%02d:%02d\t", time.tm_hour, time.tm_min, time.tm_sec);*/
}


int main(int argc, char* argv[]) {
	if (argc == 0 || argc == 1)
		printf("Please input legal path!\n");
	else
		printdir(argv[1], 0);
	return 0;
 }