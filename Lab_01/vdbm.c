/*----------------------------------------
I declare that this piece of work which is the basis 
for recognition of achieving learning outcomes in the
OPS1 course was completed on my own.
Muratcan Akcay 303026
-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH 32
#define lock "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"

#define ERR(source) (printf("Error: %s\n", source), exit(EXIT_FAILURE))

void usage(char* pname){
	fprintf(stderr,"USAGE:%s [create] [show] [-r|l] option(s) \n", pname);
	exit(EXIT_FAILURE);
}

void create_db(int argc, char** argv)
{
    FILE* s1;
    int i, offset = 0;    
    char option[MAX_LENGTH+1];
    
    if (0 == strcmp(argv[2], "-l")) // argv offset if -l used
        offset = 1;
           
    if ((s1 = fopen(argv[2+offset], "r")) != NULL) ERR("File already exists!");    
    if ((s1 = fopen(argv[2+offset], "w+")) == NULL) ERR("fopen");
    
    for (i = 3+offset; i < argc; i++)
    {
        strcpy(option, argv[i]);
        if (strlen(option) > MAX_LENGTH) ERR("Option too long.");
        fprintf(s1, "%-32s%-32d\n", option, 0);
    }

    if (offset) fprintf(s1, "%s", lock);
    
    if (fclose(s1)) ERR("fclose");

    exit(EXIT_SUCCESS);
}

void show_db(char* filename)
{
    FILE* s1;
    char line[(MAX_LENGTH * 2) + 2];
    int lock_flag = 0;

    if ((s1 = fopen(filename, "r")) == NULL) ERR("File does not exist!");
    
    printf("Database filename: %s\n",filename);
    while (fgets(line, ((MAX_LENGTH * 2) + 2), s1) != NULL)
    {  
        lock_flag = (strcmp(line, lock) == 0) ? 1 : 0;  // if XXX encountered file is locked
        printf("%s", line);
    }

    if (lock_flag) printf("Database is locked\n");
    else printf("Database is unlocked\n");

    if (fclose(s1)) ERR("fclose");

    exit(EXIT_SUCCESS);
}

void vote(char** argv)
{  
    int offset = 0;
    int diff = 1;
    if (0 == strcmp(argv[2],"-r")) // argv offset if -r used
    {          
        offset = 1;
        diff = -1;
    }

    FILE* s1;
    char option[MAX_LENGTH + 2];
    char votes[MAX_LENGTH + 2];
    char line[(MAX_LENGTH * 2) + 2]; // to be able to hold XXXXX if file is locked
    int line_no = 1;
    int found_flag = 0;
    int append_flag = 1;
    long l_votes;  

    if ((s1 = fopen(argv[2+offset], "r+")) == NULL) ERR("File does not exist!");

    strcpy(option, argv[3+offset]);
    if (strlen(option) > MAX_LENGTH) ERR("Option too long.");

    while (fscanf(s1, "%s", line) == 1) // read line until whitespace or \n
    {
        if (strncmp(line, lock, MAX_LENGTH * 2) == 0) // if line is XXXXX file is locked and at EOF
        {
            append_flag = 0;
            break;
        }
        
        fscanf(s1, "%s", votes); // read votes
        
        if (strcmp(line, option) == 0)  // if option matchs the line remember the votes
        {
            found_flag = 1;
            l_votes = strtol(votes, (char**) NULL, 10);
        }
         
        if (!found_flag) line_no++; // keeping track of line number of the matching option
    } 

    if (found_flag)
    {
        if(fseek(s1, (65 * (line_no - 1)) + 32, SEEK_SET)) ERR("fseek"); // move to the position
        fprintf(s1, "%ld", l_votes + diff);  // update votes
    }
    else
        if (append_flag) fprintf(s1, "%-32s%-32d\n", option, 0);   // if option not found add it
        
    if (fclose(s1)) ERR("fclose");    
   
    exit(EXIT_SUCCESS);
}


int main(int argc, char** argv) 
{	
    if (1 == argc) usage(argv[0]);
    
    if (0 == strcmp(argv[1], "create"))
        create_db(argc, argv);
    else if (0 == strcmp(argv[1], "show"))
        show_db(argv[2]);
    else if (0 == strcmp(argv[1], "vote"))
         vote(argv); 
    else usage(argv[0]);

    return EXIT_SUCCESS;
}