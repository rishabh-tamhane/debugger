
#include "error_proof.h"
#include "deet.h"
#include "evaluation.h"
#include "command.h"



int main(int argc, char *argv[]) {
    // TO BE IMPLEMENTED
    // Remember: Do not put any functions other than main() in this file.

    /*LOGGING DEET START*/
    log_startup();
    //installing the signal handler for SIGCHILD
    if(signal(SIGCHLD,sigchild_handler)==SIG_ERR){
        unix_error("signal error");
    }

    
    //installing the signal handler for SIGINT
    
    if(signal(SIGINT,sigint_handler)==SIG_ERR)
        unix_error("signal error");
    
    extern int finished_flag;

    extern char *input;
    size_t len=0;
    int id=0, index=0;
    pid_t pid_tmp=0;
    int deet_flag=0;
    if(argc>1){
    if(strcmp(argv[1],"-p")==0)
        deet_flag=1;
    }
    

    //signal masking declarations
    sigset_t mask, prev_mask;
    char input_cpy[MAXLINE],input_cpy1[MAXLINE];
    //cmd_list is the pointer to a block of a boxes which hold information about commands
 

    //error handling for non allocation pending

    char *execve_arguments[MAXARGS];

    //signal masking variables
    //sigset_t mask,prev_mask;

    //initialising the flags to 0 to show that no command has been received so far
    for(int i=0;i<MAXARGS;i++){
        cmd_list[i].r_status=PSTATE_NONE;
    }



    while(1){
        /*LOGGING DEET PROMPT*/
        log_prompt();
        if(deet_flag==0){
        fprintf(stdout,"deet> ");
        fflush(stdout);
        }
        ssize_t cmd_len= getline(&input,&len,stdin); 
        log_input(input);   

        if(cmd_len ==-1){
            fprintf(stderr, "%d Unable to read: %s\n",finished_flag,strerror(errno));
            fflush(stdout);
            break;
        }      
        else if(cmd_len==1 && input[0]=='\n'){
            continue;
        } 


        if(input[cmd_len-1]=='\n'){
            input[cmd_len-1]=' ';
        }

        strcpy(input_cpy,input);
        strcpy(input_cpy1,input);


        Sigemptyset(&mask);
        Sigaddset(&mask,SIGCHLD);
        Sigaddset(&mask,SIGINT);
        Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

        int tmp_index=0,pending_tasks=0;



        while(tmp_index<MAXARGS){
            
            if(cmd_list[tmp_index].r_status!=PSTATE_NONE && cmd_list[tmp_index].r_status!=PSTATE_DEAD)
                pending_tasks++;
            tmp_index++;
        }

        Sigprocmask(SIG_SETMASK,&prev_mask,NULL);

        if(pending_tasks>0)
            finished_flag=0;
        else 
            finished_flag=1;

        
        if((id=evaluate(input))==NOMATCH_NUM){ //incorrect input 
            /*LOGGING USER INPUT ERROR*/
            log_error("invalid arguments or wrong instructions");
            fprintf(stdout,"?\n");
            fflush(stdout);
        }
        else if(id==HELP_NUM)              //help
            help_print();
        else if(id==QUIT_NUM){             //quit
            //Need to wait and free stuff before exiting 
            
            break;
        }
        else if(id==RUN_NUM){            // run 
            index=0;
            Sigemptyset(&mask);
            Sigaddset(&mask,SIGCHLD);
            Sigaddset(&mask,SIGINT);
            Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

            int dead_index=0;
            while(dead_index<MAXARGS){
                if(cmd_list[dead_index].r_status==PSTATE_KILLED || cmd_list[dead_index].r_status==PSTATE_DEAD)
                    cmd_list[dead_index].r_status=PSTATE_NONE;
                dead_index++;
            }
            while(cmd_list[index].r_status!=PSTATE_NONE)
                index++;
            
            Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
            gettoken(input_cpy,execve_arguments);
            /*
            if(tok_flag==1){
                fprintf(stdout,"Tokenization done \n");
                fprintf(stdout,"The first token is %s\n",execve_arguments[1]);
                fprintf(stdout,"%s",execve_arguments[1]); 
                fflush(stdout);
            }
            */

            if((pid_tmp = Fork())==0){
                //fprintf(stdout,"child forked the pid is %d",getpid());
                //fflush(stdout);

                dup2(STDERR_FILENO,STDOUT_FILENO);
                //fprintf(stdout,"duping done\n");
                
                ptrace(PTRACE_TRACEME);
                //fprintf(stdout,"ptrace done\n");
                //char *args1[]={"echo","a","b","c",NULL};
                //execvp(args1[0],args1);
                execvp(execve_arguments[1],execve_arguments+1); 
                //fprintf(stdout,"Reached here now");
                /*execve(path,execve_arguments+1,NULL) */
            }
            else{
                
                //updating the cmd list
                Sigemptyset(&mask);
                Sigaddset(&mask,SIGCHLD);
                Sigaddset(&mask,SIGINT);
                Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

                char temp_str[MAXLINE]="";
                strcat(temp_str,execve_arguments[0]);
                for(int i=1;execve_arguments[i]!=NULL;i++){
                        strcat(temp_str," ");
                        strcat(temp_str,execve_arguments[i]);
                }

                /*LOGGING STATE CHANGE*/
                log_state_change(pid_tmp,cmd_list[index].r_status , PSTATE_RUNNING, 0);
                cmd_list[index].deet_id=index;
                cmd_list[index].p_id=pid_tmp;
                cmd_list[index].t_status='T';
                cmd_list[index].r_status=PSTATE_RUNNING;
                strcpy(cmd_list[index].command,strip_command_line(temp_str)+1);
                finished_flag=0;
                fprintf(stdout,"%d\t%d\t%c\t%s\t\t%s\n",cmd_list[index].deet_id,cmd_list[index].p_id,cmd_list[index].t_status,convert_status(cmd_list[index].r_status),cmd_list[index].command);
                //fprintf(stdout,"parent reporting, the child pid should be %d",pid_tmp);
                fflush(stdout);

                while(cmd_list[index].r_status!=PSTATE_STOPPED){
                    sigsuspend(&prev_mask);
                }

                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                
                    //need to get this removed later 
                    /*
                while((pid_wt=waitpid(-1,&status,0))>0){
                if(WIFEXITED(status)){
                    fprintf(stdout,"child %d terminated normally with exit status=%d\n",pid_wt,WEXITSTATUS(status));
                    fflush(stdout);
                }
                else{
                    fprintf(stdout,"child %d terminated abnormally\n",pid_wt);
                    fflush(stdout);
                }
                } */
            }


        }
        else if(id==SKIP_NUM){
            continue;
        }
    }  


    free(input);

    /* LOGGING DEET END*/
    log_shutdown();
    return 0;  
    //abort();
}
