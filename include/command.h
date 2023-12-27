#ifndef __COMMAND_STRUCT__
#define __COMMAND_STRUCT__

#include<sys/types.h>

typedef struct CMD_BOX{
    int deet_id;                         //deet_id
    pid_t p_id;                          //process id  
    char t_status;                       //possible to trace
    volatile int r_status;                   // running status of the child
    int  e_id;                           // exit flag of the child   
    char command[8192];             //command   
} cmd_box;

extern cmd_box cmd_list[128];

#endif /* __COMMAND_STRUCT__ */
/* $end command.h */