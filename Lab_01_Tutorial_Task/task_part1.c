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
        
    char path[MAX_PATH];
    
    if (getcwd(path, MAX_PATH) == NULL) ERR("getcwd");
    if (NULL== (dirp = opendir("."))) ERR("opendir");
    
    printf("Files in %s:\n", path);

    do 
    {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL)
            printf("%s\n", dp->d_name);                                  
        
    } while (dp != NULL);

    if (errno != 0) ERR("readdir");
    if (closedir(dirp) != 0) ERR("closedir");    
}


int main(int argc, char** argv) 
{	
    
    
    if (1 == argc)
    {
       scan_dir();
    }
       
    return EXIT_SUCCESS;
}