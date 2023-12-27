#include "error_proof.h"
#include "deet.h"
#include "command.h"


int finished_flag=0;
char *input=NULL;

/* $begin unixerror */
void unix_error(char *msg) /* Unix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}
/* $end unixerror */

void app_error(char *msg) /* Application error */
{
    fprintf(stderr, "%s\n", msg);
    exit(0);
}



/* $begin forkwrapper */
pid_t Fork(void) 
{
    pid_t pid;

    if ((pid = fork()) < 0)
	unix_error("Fork error");
    return pid;
}
/* $end forkwrapper */

void Execvp(const char *filename, char *const argv[]) 
{
    if (execvp(filename, argv) < 0)
	unix_error("Execve error");
}



/*
char *Fgets(char *ptr, int n, FILE *stream) 
{
    char *rptr;

    if (((rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream))
	    app_error("Fgets error");

    return rptr;
}
*/


//SIO Functions
ssize_t sio_puts(char s[]) /* Put string */
{
    return write(STDOUT_FILENO, s, sio_strlen(s)); 
}

/* sio_strlen - Return length of string (from K&R) */
static size_t sio_strlen(char s[])
{
    int i = 0;

    while (s[i] != '\0')
        ++i;
    return i;
}

void sio_error(char s[]) /* Put error message and exit */
{
    sio_puts(s);
    _exit(1);                                      
}

ssize_t Sio_puts(char s[])
{
    ssize_t n;
  
    if ((n = sio_puts(s)) < 0)
	sio_error("Sio_puts error");
    return n;
}

ssize_t sio_putl(long v) /* Put long */
{
    char s[128];
    
    sio_ltoa(v, s, 10); /* Based on K&R itoa() */  //line:csapp:sioltoa
    return sio_puts(s);
}

ssize_t Sio_putl(long v)
{
    ssize_t n;
  
    if ((n = sio_putl(v)) < 0)
	sio_error("Sio_putl error");
    return n;
}

static void sio_reverse(char s[])
{
    int c, i, j;

    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}


/* sio_ltoa - Convert long to base b string (from K&R) */
static void sio_ltoa(long v, char s[], int b) 
{
    int c, i = 0;
    int neg = v < 0;

    if (neg)
	v = -v;

    do {  
        s[i++] = ((c = (v % b)) < 10)  ?  c + '0' : c - 10 + 'a';
    } while ((v /= b) > 0);

    if (neg)
	s[i++] = '-';

    s[i] = '\0';
    sio_reverse(s);
}
//Signal Handler Functions

void sigchild_handler(int sig){
    log_signal(SIGCHLD);
    int old_errno=errno;
    int status;
    int index=0;
    sigset_t mask,prev_mask;
    //int linelen=0;
    //char buffer[MAXLINE+200];
    pid_t pidw;
    if((pidw=waitpid(-1,&status, WNOHANG | WUNTRACED | WCONTINUED ))<0)
        sio_error("waitpid_error");
    /*
    if(pidw==0)
        Sio_puts("Handler returned empty which means you didnt receive any signal");
    else
        Sio_puts("Handler has something\n");
    */
    

    Sigemptyset(&mask);
    Sigaddset(&mask,SIGINT);
    Sigprocmask(SIG_BLOCK,&mask,&prev_mask);
        
    if(WIFSTOPPED(status)){
        int status_number=WSTOPSIG(status);
        /*
        Sio_puts("Child received stop signal. updating the cmd_list box\n");
        */

    while((cmd_list[index].p_id)!=pidw )
        index++;
    /*LOGGING STATE CHANGE*/
    log_state_change(cmd_list[index].p_id,cmd_list[index].r_status , PSTATE_STOPPED, 0);
    cmd_list[index].r_status= PSTATE_STOPPED;
    /*Write is async*/
    for(int i=0;i<=5;i++){

        if(i==0){
            long message_num=cmd_list[index].deet_id;
            Sio_putl(message_num);
            Sio_puts("\t");
        }

        if(i==1){
            long message_num=cmd_list[index].p_id;
            Sio_putl(message_num);
            Sio_puts("\t");
        }

        if(i==2){
            char message[2];
            message[0]=cmd_list[index].t_status;
            message[1]='\0';
            Sio_puts(message);
            //write(STDOUT_FILENO,(char*)message,1);
            Sio_puts("\t");
        }

        if(i==3){
            if(cmd_list[index].r_status==PSTATE_RUNNING)
                Sio_puts("running");
            else if(cmd_list[index].r_status==PSTATE_CONTINUING)
                Sio_puts("continuing");
            else if(cmd_list[index].r_status==PSTATE_STOPPED)
                Sio_puts("stopped");
            else if(cmd_list[index].r_status==PSTATE_STOPPING)
                Sio_puts("stopping");
            else if(cmd_list[index].r_status==PSTATE_KILLED)
                Sio_puts("dead");
            else if(cmd_list[index].r_status==PSTATE_DEAD)
                Sio_puts("dead");
            Sio_puts("\t");
        }

        if(i==4){
            char message[20];
            if(cmd_list[index].r_status==PSTATE_KILLED || cmd_list[index].r_status==PSTATE_DEAD){
                char *hex=message;
                char hex_full[]="0123456789ABCDEF";
                int shift = sizeof(status_number)*8-4;
                int skipzeros=1;

                for(int i=0;i<8;i++){
                    char hd=hex_full[(status_number>>shift) & 0xF];

                    if(hd != '0' || !skipzeros){
                        *hex++ = hd;
                        skipzeros=0;
                    }

                    shift -=4;
                }

                *hex='\0';
            
            Sio_puts("0x");
            if(cmd_list[index].e_id==0)
                Sio_puts("0");
            else    
                Sio_puts(message);
            Sio_puts("\t");
            }
            else{
                Sio_puts("\t");
            }
        }
        if(i==5)
            Sio_puts(cmd_list[index].command);



    }
    Sio_puts("\n");
    //linelen=sprintf(buffer,"%d\t%d\t%c\tstopped\t%s\n",cmd_list[index].deet_id,cmd_list[index].p_id,cmd_list[index].t_status,cmd_list[index].command);  

    //fprintf(stdout,"The stop signal came because of %d\n",WSTOPSIG(status));
    //fflush(stdout);
    //write(STDOUT_FILENO,buffer,linelen);

    }

    if(WIFEXITED(status)){
        //Sio_puts("Child exited\n");
        int status_number=WEXITSTATUS(status);


        while((cmd_list[index].p_id)!=pidw )
            index++;
        /*LOGGING STATE CHANGE*/
        log_state_change(cmd_list[index].p_id,cmd_list[index].r_status , PSTATE_DEAD, WEXITSTATUS(status));
        cmd_list[index].r_status= PSTATE_DEAD;
        cmd_list[index].e_id=WEXITSTATUS(status);

    for(int i=0;i<=5;i++){

        if(i==0){
            long message_num=cmd_list[index].deet_id;
            Sio_putl(message_num);
            Sio_puts("\t");
        }

        if(i==1){
            long message_num=cmd_list[index].p_id;
            Sio_putl(message_num);
            Sio_puts("\t");
        }

        if(i==2){
            char message[2];
            message[0]=cmd_list[index].t_status;
            message[1]='\0';
            Sio_puts(message);
            //write(STDOUT_FILENO,(char*)message,1);
            Sio_puts("\t");
        }

        if(i==3){
            if(cmd_list[index].r_status==PSTATE_RUNNING)
                Sio_puts("running");
            else if(cmd_list[index].r_status==PSTATE_CONTINUING)
                Sio_puts("continuing");
            else if(cmd_list[index].r_status==PSTATE_STOPPED)
                Sio_puts("stopped");
            else if(cmd_list[index].r_status==PSTATE_STOPPING)
                Sio_puts("stopping");
            else if(cmd_list[index].r_status==PSTATE_KILLED)
                Sio_puts("dead");
            else if(cmd_list[index].r_status==PSTATE_DEAD)
                Sio_puts("dead");
            Sio_puts("\t");
        }

        if(i==4){
            char message[20];
            if(cmd_list[index].r_status==PSTATE_KILLED || cmd_list[index].r_status==PSTATE_DEAD){
                char *hex=message;
                char hex_full[]="0123456789ABCDEF";
                int shift = sizeof(status_number)*8-4;
                int skipzeros=1;

                for(int i=0;i<8;i++){
                    char hd=hex_full[(status_number>>shift) & 0xF];

                    if(hd != '0' || !skipzeros){
                        *hex++ = hd;
                        skipzeros=0;
                    }

                    shift -=4;
                }

                *hex='\0';
            
            Sio_puts("0x");
            if(cmd_list[index].e_id==0)
                Sio_puts("0");
            else          
                Sio_puts(message);
            Sio_puts("\t");

            }
            else{
                Sio_puts("\t");
            }
        }
        if(i==5)
            Sio_puts(cmd_list[index].command);
    }
        Sio_puts("\n");

        //linelen=sprintf(buffer,"%d\t%d\t%c\tdead\t%#08x\t%s\n",cmd_list[index].deet_id,cmd_list[index].p_id,cmd_list[index].t_status,cmd_list[index].e_id,cmd_list[index].command);  

        //fprintf(stdout,"The stop signal came because of %d\n",WEXITSTATUS(status));
        //fflush(stdout);



        //write(STDOUT_FILENO,buffer,linelen);
    }



    if(WIFCONTINUED(status)){
        //Sio_puts("Child e");
        int status_number=0;

        while((cmd_list[index].p_id)!=pidw )
            index++;
        /*LOGGING STATE CHANGE*/
        log_state_change(cmd_list[index].p_id,cmd_list[index].r_status , PSTATE_RUNNING, 0);
        cmd_list[index].r_status= PSTATE_RUNNING;

        for(int i=0;i<=5;i++){

            if(i==0){
                long message_num=cmd_list[index].deet_id;
                Sio_putl(message_num);
                Sio_puts("\t");
            }

            if(i==1){
                long message_num=cmd_list[index].p_id;
                Sio_putl(message_num);
                Sio_puts("\t");
            }

            if(i==2){
                char message[2];
                message[0]=cmd_list[index].t_status;
                message[1]='\0';
                Sio_puts(message);
                //write(STDOUT_FILENO,(char*)message,1);
                Sio_puts("\t");
            }

            if(i==3){
                if(cmd_list[index].r_status==PSTATE_RUNNING)
                    Sio_puts("running");
                else if(cmd_list[index].r_status==PSTATE_CONTINUING)
                    Sio_puts("continuing");
                else if(cmd_list[index].r_status==PSTATE_STOPPED)
                    Sio_puts("stopped");
                else if(cmd_list[index].r_status==PSTATE_STOPPING)
                    Sio_puts("stopping");
                else if(cmd_list[index].r_status==PSTATE_KILLED)
                    Sio_puts("dead");
                else if(cmd_list[index].r_status==PSTATE_DEAD)
                    Sio_puts("dead");
                Sio_puts("\t");
            }

            if(i==4){
                char message[20];
                if(cmd_list[index].r_status==PSTATE_KILLED || cmd_list[index].r_status==PSTATE_DEAD){
                    char *hex=message;
                    char hex_full[]="0123456789ABCDEF";
                    int shift = sizeof(status_number)*8-4;
                    int skipzeros=1;

                    for(int i=0;i<8;i++){
                        char hd=hex_full[(status_number>>shift) & 0xF];

                        if(hd != '0' || !skipzeros){
                            *hex++ = hd;
                            skipzeros=0;
                        }

                        shift -=4;
                    }

                    *hex='\0';
                
                Sio_puts("0x");
                if(cmd_list[index].e_id==0)
                    Sio_puts("0");
                else          
                    Sio_puts(message);
                Sio_puts("\t");

                }
                else{
                    Sio_puts("\t");
                }
            }
            if(i==5)
                Sio_puts(cmd_list[index].command);
        }

    //linelen=sprintf(buffer,"%d\t%d\t%c\trunning\t%s\n",cmd_list[index].deet_id,cmd_list[index].p_id,cmd_list[index].t_status,cmd_list[index].command);  

    //fprintf(stdout,"The continuation signal came because of %d\n",WSTOPSIG(status));
    //fflush(stdout);
    //write(STDOUT_FILENO,buffer,linelen);
    }
    


    if(WIFSIGNALED(status)){
        //Sio_puts("Child e");
        int status_number=WTERMSIG(status);
        while((cmd_list[index].p_id)!=pidw )
            index++;
        cmd_list[index].e_id=status_number;

        /*LOGGING STATE CHANGE*/
        log_state_change(cmd_list[index].p_id,cmd_list[index].r_status , PSTATE_DEAD, status_number);
        cmd_list[index].r_status= PSTATE_DEAD;

        for(int i=0;i<=5;i++){

            if(i==0){
                long message_num=cmd_list[index].deet_id;
                Sio_putl(message_num);
                Sio_puts("\t");
            }

            if(i==1){
                long message_num=cmd_list[index].p_id;
                Sio_putl(message_num);
                Sio_puts("\t");
            }

            if(i==2){
                char message[2];
                message[0]=cmd_list[index].t_status;
                message[1]='\0';
                Sio_puts(message);
                //write(STDOUT_FILENO,(char*)message,1);
                Sio_puts("\t");
            }

            if(i==3){
                if(cmd_list[index].r_status==PSTATE_RUNNING)
                    Sio_puts("running");
                else if(cmd_list[index].r_status==PSTATE_CONTINUING)
                    Sio_puts("continuing");
                else if(cmd_list[index].r_status==PSTATE_STOPPED)
                    Sio_puts("stopped");
                else if(cmd_list[index].r_status==PSTATE_STOPPING)
                    Sio_puts("stopping");
                else if(cmd_list[index].r_status==PSTATE_KILLED)
                    Sio_puts("dead");
                else if(cmd_list[index].r_status==PSTATE_DEAD)
                    Sio_puts("dead");
                Sio_puts("\t");
            }

            if(i==4){
                char message[20];
                if(cmd_list[index].r_status==PSTATE_KILLED || cmd_list[index].r_status==PSTATE_DEAD){
                    char *hex=message;
                    char hex_full[]="0123456789ABCDEF";
                    int shift = sizeof(status_number)*8-4;
                    int skipzeros=1;


                    for(int i=0;i<8;i++){
                        char hd=hex_full[(status_number>>shift) & 0xF];

                        if(hd != '0' || !skipzeros){
                            *hex++ = hd;
                            skipzeros=0;
                        }

                        shift -=4;
                    }

                    *hex='\0';
                
                Sio_puts("0x");
                if(cmd_list[index].e_id==0)
                    Sio_puts("0");
                else          
                    Sio_puts(message);
                Sio_puts("\t");

                }
                else{
                    Sio_puts("\t");
                }
            }
            if(i==5)
                Sio_puts(cmd_list[index].command);
        }

        Sio_puts("\n");

        int tmp_index=0,pending_tasks=0;

        while(tmp_index<MAXARGS){
            
            if(cmd_list[tmp_index].r_status!=PSTATE_NONE && cmd_list[tmp_index].r_status!=PSTATE_DEAD)
                pending_tasks++;
            tmp_index++;
        }

        if(pending_tasks>0)
            finished_flag=0;
        else 
            finished_flag=1;
    //linelen=sprintf(buffer,"%d\t%d\t%c\trunning\t%s\n",cmd_list[index].deet_id,cmd_list[index].p_id,cmd_list[index].t_status,cmd_list[index].command);  

    //fprintf(stdout,"The continuation signal came because of %d\n",WSTOPSIG(status));
    //fflush(stdout);
    //write(STDOUT_FILENO,buffer,linelen);
    }
    
    //fprintf(stdout,"%d:%d:",status,pidw);
    //fprintf(stdout,"%d:%d:",WIFSTOPPED(status),status);

    Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
    errno = old_errno;
}


void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    if (sigprocmask(how, set, oldset) < 0)
	unix_error("Sigprocmask error");
    return;
}

void Sigemptyset(sigset_t *set)
{
    if (sigemptyset(set) < 0)
	unix_error("Sigemptyset error");
    return;
}

void Sigfillset(sigset_t *set)
{ 
    if (sigfillset(set) < 0)
	unix_error("Sigfillset error");
    return;
}

void Sigaddset(sigset_t *set, int signum)
{
    if (sigaddset(set, signum) < 0)
	unix_error("Sigaddset error");
    return;
}

void Sigdelset(sigset_t *set, int signum)
{
    if (sigdelset(set, signum) < 0)
	unix_error("Sigdelset error");
    return;
}

int Sigismember(const sigset_t *set, int signum)
{
    int rc=sigismember(set, signum);
    if (rc < 0)
	    unix_error("Sigismember error");
    return rc;
}

int Sigsuspend(const sigset_t *set)
{
    int rc = sigsuspend(set); /* always returns -1 */
    if (errno != EINTR)
        unix_error("Sigsuspend error");
    return rc;
}


void Kill(pid_t pid, int signum) 
{
    int rc;

    if ((rc = kill(pid, signum)) < 0)
	unix_error("Kill error");
}



void sigint_handler(int sig){
    log_signal(SIGINT);
    int old_errno=errno;
    sigset_t mask,prev_mask;
    pid_t kill_pid;

    for(int i=0;i<MAXARGS;i++){

        Sigemptyset(&mask);
        Sigaddset(&mask,SIGCHLD);
        Sigprocmask(SIG_BLOCK,&mask,&prev_mask);
        int flag= (cmd_list[i].r_status!=PSTATE_DEAD && cmd_list[i].r_status!=PSTATE_NONE && cmd_list[i].r_status!=PSTATE_KILLED);
        if(flag==1){
            kill_pid=cmd_list[i].p_id;
        }

        if(flag){
            Kill(kill_pid,SIGKILL);
            //Sio_puts("current status to killed yet to write");
            //WRITE THE STOPPED TO DEAD line
            int index=0;
            while((cmd_list[index].p_id)!=kill_pid)
                index++;
            
            /*LOGGING STATE CHANGE*/
            log_state_change(cmd_list[index].p_id,cmd_list[index].r_status , PSTATE_KILLED, 0);
            cmd_list[index].r_status= PSTATE_KILLED;

            for(int i=0;i<=5;i++){

                if(i==0){
                    long message_num=cmd_list[index].deet_id;
                    Sio_putl(message_num);
                    Sio_puts("\t");
                }

                if(i==1){
                    long message_num=cmd_list[index].p_id;
                    Sio_putl(message_num);
                    Sio_puts("\t");
                }

                if(i==2){
                    char message[2];
                    message[0]=cmd_list[index].t_status;
                    message[1]='\0';
                    Sio_puts(message);
                    //write(STDOUT_FILENO,(char*)message,1);
                    Sio_puts("\t");
                }

                if(i==3){
                    if(cmd_list[index].r_status==PSTATE_RUNNING)
                        Sio_puts("running");
                    else if(cmd_list[index].r_status==PSTATE_CONTINUING)
                        Sio_puts("continuing");
                    else if(cmd_list[index].r_status==PSTATE_STOPPED)
                        Sio_puts("stopped");
                    else if(cmd_list[index].r_status==PSTATE_STOPPING)
                        Sio_puts("stopping");
                    else if(cmd_list[index].r_status==PSTATE_KILLED)
                        Sio_puts("dead");
                    else if(cmd_list[index].r_status==PSTATE_DEAD)
                        Sio_puts("dead");
                    Sio_puts("\t");
                }

                if(i==4){
                    Sio_puts("\t");

                    }

                if(i==5)
                    Sio_puts(cmd_list[index].command);
            }

            Sio_puts("\n");

        }

        Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
    }



    Sigemptyset(&mask);
    Sigaddset(&mask,SIGCHLD);
    Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

    while(finished_flag!=1){
        sigsuspend(&prev_mask);
    }

    Sigprocmask(SIG_SETMASK,&prev_mask,NULL);

    errno = old_errno;

    free(input);
    log_shutdown();
    //Kill(getpid(),SIGKILL);
    exit(0);

}

