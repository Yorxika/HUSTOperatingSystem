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
		//打开目录不成功
		printf("Open dir failed!\n");
		return;
	}

	printf("%s\n\n", dir);  //打印当前路径

	chdir(dir);  //进入目录
		while ((entry = readdir(dp)) != NULL) {
			//读到一个目录项
			lstat(entry->d_name,statbuf);  //以该目录项的名字为参数,调用lstat得到该目录项的相关信息; 
			if (S_ISDIR(statbuf->st_mode)) {  //如果是一个目录
				if ((!strcmp(entry->d_name, ".")) || (!strcmp(entry->d_name, "..")))
					continue;
				else {
					//不是. 或者 .. 打印目录项的深度、目录名等信息 递归调用printdir,打印子目录的信息,其中的depth+4; 
					//printf("Current dir  deepth is %d !", deepth);
					printf("depth: %d \t", deepth);
					printInfo(*statbuf);
					printf("%s\n", entry->d_name);
					printdir(entry->d_name, deepth + 4);
				}
			}
			else {
				//打印文件的深度、文件名等信息
				//printf("Current file deepth is %d !", deepth);
				printf("depth: %d \t", deepth);
				printInfo(*statbuf);
				printf("%s\n", entry->d_name);
			}
	}
	chdir("../");
	closedir(dp);
}

void printFileDetails(mode_t mode){

	char str[11] = { 0 };

	str[0] = S_ISDIR(mode) ? 'd' : '-';   // 目录
	str[0] = S_ISCHR(mode) ? 'c' : '-';  //  字符设备
	str[0] = S_ISBLK(mode) ? 'b' : '-';   // 块设备
	str[0] = S_ISLNK(mode) ?  'l' : '-';  //链接
	str[0] = S_ISFIFO(mode) ? 'f' : '-';   //管道文件

	str[1] = mode & S_IRUSR ? 'r' : '-';   // 所有者
	str[2] = mode & S_IWUSR ? 'w' : '-';
	str[3] = mode & S_IXUSR ? 'x' : '-';

	str[4] = mode & S_IRGRP ? 'r' : '-';   // 群组
	str[5] = mode & S_IWGRP ? 'w' : '-'; 
	str[6] = mode & S_IXGRP ? 'x' : '-';

	str[7] = mode & S_IROTH ? 'r' : '-';   // 其他
	str[8] = mode & S_IWOTH ? 'w' : '-';
	str[9] = mode & S_IXOTH ? 'x' : '-';

	printf("%s\t", str);
}

void printInfo(struct stat state){

	printFileDetails(state.st_mode);

	//硬连接的数目
	printf("%ld\t", state.st_nlink);

	//用户名
	struct passwd* pwd;
	if ((pwd = getpwuid(state.st_uid)) != NULL) {
		printf("%s\t", pwd->pw_name);
	}
	else {
		printf("%d\t", state.st_uid);
	}

	//群组
	struct group* grp;
	if ((grp = getgrgid(state.st_gid)) != NULL) {
		printf("%s\t", grp->gr_name);
	}
	else
		printf("%8d\t", state.st_gid);


	//文件大小
	if(state.st_size < 1024)
		printf("%lfB\t", (double)state.st_size);
	else if (state.st_size < 1024 * 1024) 
		printf("%lfK\t", (double)state.st_size / 1024);
	else if (state.st_size < 1024 * 1024 * 1024) 
		printf("%lfM\t", (double)state.st_size / 1024 / 1024);
	else
		printf("%lfG\t", (double)state.st_size / 1024 / 1024 / 1024);

	//修改时间
	time_t t = state.st_mtime;
	struct tm time;
	localtime_r(&t, &time);
	printf("modified time:%d/", time.tm_year + 1900);
	printf("%d/", time.tm_mon + 1);
	printf("%d\t", time.tm_mday);
	//printf("%.12s ", ctime(state.st_mtime) + 4);

}


int main(int argc, char* argv[]) {
	if (argc == 0 || argc == 1)
		printf("Please input legal path!\n");
	else
		printdir(argv[1], 0);
	return 0;
 }