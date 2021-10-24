#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <errno.h>
#include <unistd.h>
#define MAX_PATH 50
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))


int scan_dir (char* path, int limit){
	DIR *dirp;
	struct dirent *dp;
	struct stat filestat;
	int dirs=0,files=0,links=0,other=0;
	long result = 0;
	if(NULL == (dirp = opendir(path))) ERR("opendir");
	do {
		errno = 0;
		if ((dp = readdir(dirp)) != NULL) {
			if (lstat(dp->d_name, &filestat)) ERR("lstat");
			if (S_ISDIR(filestat.st_mode)) // is a directory
			{
				dirs++;

			} 
			else if (S_ISREG(filestat.st_mode)) // is a file
			{
				files++;
				FILE* current = fopen(dp->d_name, "r");
				fseek(current, 0L, SEEK_END);
				long size = ftell(current); result += size;
				fseek(current, 0L, SEEK_SET);
				fclose(current);
				printf("Size of this file is %ld\n", size);
			} 
			else if (S_ISLNK(filestat.st_mode)) // is a link
			{
				links++;
			} 
			else other++;
		}
	} while (dp != NULL);
	if (errno != 0) ERR("readdir");
	if(closedir(dirp)) ERR("closedir");
	printf("Files: %d, Dirs: %d, Links: %d, Other: %d\nSum of sizes of all files is %ld\n",files,dirs,links,other, result);
	if(result > limit) return 0;
	else return 1;
}
int main(int argc, char** argv)
{
	if(argc % 2 == 0)
		return EXIT_FAILURE;
	printf("Hello, world!\n");
	char path[MAX_PATH];
	if(getcwd(path, MAX_PATH)==NULL) ERR("getcwd");
	for(int i = 1; i < argc; i+=2)
	{
		int limit = atoi(argv[1+i]);
		if(chdir(argv[i]))ERR("chdir");
		
		if(scan_dir(argv[i], limit) == 0) {
			printf("THIS PATH IS CORRECT:%s\nWriting to file out.txt...\n",argv[i]);
			FILE *f = fopen("out.txt", "a+");
			fprintf(f, "%s\n", argv[i]);
			fclose(f);
		}
		if(chdir(path))ERR("chdir");
	}
	
	
	return EXIT_SUCCESS;
}