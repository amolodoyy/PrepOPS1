#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>
#include <string.h>
#include <unistd.h>
#define MAXFD 30
#define ENV_VAR "L1_LOGFILE"
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))
long max_file_size = 0;

FILE* restrict destination;
char* env_value;
int walk(const char *name, const struct stat *s, int type, struct FTW *f)
{

        switch (type){
                case FTW_DNR: { 
                    fprintf(destination,"Directory cannot be read.\n");
                }
                break;
                case FTW_D:
                {   
                    fprintf(destination,"Skipping directory...\n");
                }
                break;
                case FTW_F:
                {

                    uid_t uid = getuid();
                    if(s->st_uid != uid)
                    {
                        fprintf(destination,"File owned by a different user.\n");
                    }
                    else
                    {
                        if(s->st_size > max_file_size){
                            max_file_size = s->st_size;
                        }
                        fprintf(destination,"%s\n", name);
                        fprintf(destination,"Size of this file is %ld\n", s->st_size);
                    }
                }
                break;
                case FTW_SL: 
                {
                    fprintf(destination,"Skipping symlink...\n");
                }
                break;
                case FTW_NS: 
                {
                    fprintf(destination,"Permission denied.\n");
                }
                break;
                default : 
                    fprintf(destination,"Unknown object.\n");
        }
        return 0;
}

int main(int argc, char** argv)
{
    destination = stdout;
    env_value = getenv(ENV_VAR);
    if(env_value != NULL)
    {
        if((destination=fopen(env_value,"a+"))==NULL)ERR("fopen");
    }
    for(int i = 1; i < argc; i++)
    {
        if(nftw(argv[i],walk,MAXFD,FTW_PHYS)==0)
        {
            printf("Max file size in a directory is %ld\n", max_file_size);
            if(env_value != NULL){
                FILE *f; 
                if((f=fopen(env_value,"a"))==NULL)ERR("fopen");
                fprintf(f, "Max file size in a directory is %ld\n", max_file_size);
            }
            max_file_size = 0;
        }
        else
        {
            printf("Error\n");
            exit(EXIT_FAILURE);
        }
    }
    if(env_value != NULL)
    {
        if(fclose(destination))ERR("fclose");
    }
    return EXIT_SUCCESS;
}   