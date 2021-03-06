#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "dirent.h"

#define MAX_PATH 255

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

void usage(char* pname){
	fprintf(stderr,"USAGE:%s path size path size ..\n",pname);
	exit(EXIT_FAILURE);
}

void scan_dir()
{
    DIR* dirp;
    struct dirent* dp;
    struct stat filestat;
        
    char path[MAX_PATH];
    long total_size = 0;
    
    if (getcwd(path, MAX_PATH) == NULL) ERR("getcwd");
    if (NULL== (dirp = opendir("."))) ERR("opendir");
    
    printf("Files in %s:\n", path);

    do 
    {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL)
        {
            if (lstat(dp->d_name,&filestat) != 0) ERR("lstat");
            printf("%ld\n", filestat.st_size);
            total_size += filestat.st_size;
        }
        
    } while (dp != NULL);

    if (errno != 0) ERR("readdir");
    if (closedir(dirp) != 0) ERR("closedir");

    printf("Total size: %ld\n\n", total_size);    
}

int main(int argc, char** argv) 
{	
    char path[MAX_PATH];
    if (getcwd(path, MAX_PATH) == NULL) ERR("getcwd");
    
    if (1 == argc)
    {
       scan_dir();
       exit(EXIT_SUCCESS);
    }

    for (int i = 0; i < argc/2; i++) 
    {
        char* folder_name = argv[(2*i) + 1];
        if (chdir(folder_name) == -1)
        {
            printf("No access to folder \"%s\"\n", folder_name);
            continue;
        } 
       
        scan_dir();

        if(chdir(path)) ERR("chdir");
    }
       
    return EXIT_SUCCESS;
}