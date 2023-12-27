#include "evaluation.h"
#include "error_proof.h"
#include "command.h"
#include "deet.h"

extern int finished_flag;
//*Async Safety Requirement* : Yet to be implemented
//In case of read, I can understand but what if 
int evaluate(char *cmdline){

    //Break down the cmdline string into args
    char *word;
    sigset_t mask,prev_mask;
    // count how many words are there
    word= strtok(cmdline," ");
    if(strcmp(word,"help")==0)
        return HELP_NUM;
/*
    else if((strcmp(word,"quit")==0)){
        word = strtok(NULL," ");
        if(word== NULL)
            return QUIT_NUM;
        else 
            return NOMATCH_NUM;
    } */



    else if((strcmp(word,"run")==0)){
        word = strtok(NULL," ");
        if(word!=NULL)
            return RUN_NUM;
        else 
            return NOMATCH_NUM;
    }




    else if((strcmp(word,"show")==0)){

        Sigemptyset(&mask);
        Sigaddset(&mask,SIGCHLD);
        Sigaddset(&mask,SIGINT);
        Sigprocmask(SIG_BLOCK,&mask,&prev_mask);


        int show_cnt=0;
        word = strtok(NULL," ");
        if(word==NULL){
            for(int i=0;i<MAXARGS;i++){
                if(cmd_list[i].r_status!=PSTATE_NONE && cmd_list[i].r_status!=PSTATE_DEAD && cmd_list[i].r_status!=PSTATE_KILLED){
                    show_cnt++;
                    fprintf(stdout,"%d\t%d\t%c\t%s\t\t%s\n",cmd_list[i].deet_id,cmd_list[i].p_id,cmd_list[i].t_status,convert_status(cmd_list[i].r_status),cmd_list[i].command);
                    fflush(stdout);
                }
                else if(cmd_list[i].r_status==PSTATE_DEAD || cmd_list[i].r_status==PSTATE_KILLED){
                    show_cnt++;
                    fprintf(stdout,"%d\t%d\t%c\t%s\t%#x\t%s\n",cmd_list[i].deet_id,cmd_list[i].p_id,cmd_list[i].t_status,convert_status(cmd_list[i].r_status),cmd_list[i].e_id,cmd_list[i].command);
                    fflush(stdout);

                }
            }

        Sigprocmask(SIG_SETMASK,&prev_mask,NULL);

        if(show_cnt==0)
            return NOMATCH_NUM;
        else 
            return SKIP_NUM;
        }
        else if(check_number(word)==1 && strtok(NULL," ")==NULL){
            //check if the deetid is present in cmd_list
            int d_id=atoi(word),i=0;
            while(cmd_list[i].deet_id!=d_id && i<MAXARGS){
                i++;
            }

            //if deet id is present then print it
            if(cmd_list[i].r_status!=PSTATE_NONE){
                fprintf(stdout,"%d\t%d\t%c\t%s\t%s\n",cmd_list[i].deet_id,cmd_list[i].p_id,cmd_list[i].t_status,convert_status(cmd_list[i].r_status),cmd_list[i].command);
                fflush(stdout);
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return SKIP_NUM;
            }
            else if(i==MAXARGS){
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return NOMATCH_NUM;
            }


        }
        else{
            Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
            return NOMATCH_NUM;
        }

    }





    else if((strcmp(word,"cont")==0)){
        word = strtok(NULL," ");
        if(word==NULL)
            return NOMATCH_NUM;

        if(check_number(word)==1 && strtok(NULL," ")==NULL ){
            int d_id=atoi(word),i=0;

            Sigemptyset(&mask);
            Sigaddset(&mask,SIGCHLD);
            Sigaddset(&mask,SIGINT);
            Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

            while(cmd_list[i].deet_id!=d_id && i<MAXARGS){
                i++;
            }

            if(i==MAXARGS){
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return NOMATCH_NUM;
            }
            //if deet id was stopped by ptrace and receives continue
            else if(cmd_list[i].r_status==PSTATE_STOPPED && cmd_list[i].t_status=='T'){
                
                ptrace(PTRACE_CONT,cmd_list[i].p_id,NULL,NULL);
                /*LOGGING STATE CHANGE*/
                log_state_change(cmd_list[i].p_id,cmd_list[i].r_status , PSTATE_RUNNING, 0);
                cmd_list[i].r_status=PSTATE_RUNNING;
                fprintf(stdout,"%d\t%d\t%c\t%s\t\t%s\n",cmd_list[i].deet_id,cmd_list[i].p_id,cmd_list[i].t_status,convert_status(cmd_list[i].r_status),cmd_list[i].command);
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return SKIP_NUM;
            }

            //if deet id was stopped and then continued
            else if(cmd_list[i].r_status==PSTATE_STOPPED && cmd_list[i].t_status=='U'){
                
                Kill(cmd_list[i].p_id,SIGCONT);

                /*LOGGING STATE CHANGE*/
                log_state_change(cmd_list[i].p_id,cmd_list[i].r_status , PSTATE_CONTINUING, 0);

                cmd_list[i].r_status=PSTATE_CONTINUING;
                fprintf(stdout,"%d\t%d\t%c\t%s\t%s\n",cmd_list[i].deet_id,cmd_list[i].p_id,cmd_list[i].t_status,convert_status(cmd_list[i].r_status),cmd_list[i].command);
                /*
                while(cmd_list[i].r_status!=PSTATE_RUNNING)
                sigsuspend(&prev_mask);
                Sigprocmask(SIG_UNBLOCK,&mask,NULL);
                */
                return SKIP_NUM;
            }
            else {
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return NOMATCH_NUM;
            }

        return NOMATCH_NUM;
        }
         
        return NOMATCH_NUM;
    }


    /*
    while(word!=NULL){
        printf("Argument: %s\n",word);  
        word = strtok(NULL," "); //Get the next token
    }
    */
   else if(strcmp(word,"stop")==0){
        word = strtok(NULL," ");

        if(check_number(word)==1 && strtok(NULL," ")==NULL){
            int d_id=atoi(word),i=0;

            Sigemptyset(&mask);
            Sigaddset(&mask,SIGCHLD);
            Sigaddset(&mask,SIGINT);
            Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

            while(cmd_list[i].deet_id!=d_id && i<MAXARGS){
                i++;
            }

            if(i==MAXARGS){
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return NOMATCH_NUM;
            }
            else if(cmd_list[i].r_status==PSTATE_RUNNING){

                Kill(cmd_list[i].p_id,SIGSTOP);
                /*LOGGING STATE CHANGE*/
                log_state_change(cmd_list[i].p_id,cmd_list[i].r_status , PSTATE_STOPPING, 0);
                cmd_list[i].r_status=PSTATE_STOPPING;
                fprintf(stdout,"%d\t%d\t%c\t%s\t%s\n",cmd_list[i].deet_id,cmd_list[i].p_id,cmd_list[i].t_status,convert_status(cmd_list[i].r_status),cmd_list[i].command);

                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return SKIP_NUM;
            }
            else{
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return NOMATCH_NUM;
            }
        }

        else 
            return NOMATCH_NUM;


   }




    else if((strcmp(word,"release")==0)){
        word = strtok(NULL," ");


        if(check_number(word)==1 && strtok(NULL," ")==NULL ){
            int d_id=atoi(word),i=0;

            Sigemptyset(&mask);
            Sigaddset(&mask,SIGCHLD);
            Sigaddset(&mask,SIGINT);
            Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

            while(cmd_list[i].deet_id!=d_id && i<MAXARGS){
                i++;
            }

            if(i==MAXARGS){
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return NOMATCH_NUM;
            }
            //if deet id was stopped by ptrace and receives continue
            else if(cmd_list[i].r_status==PSTATE_STOPPED && cmd_list[i].t_status=='T'){
                
                //ptrace(PTRACE_CONT,cmd_list[i].p_id,NULL,NULL);
                ptrace(PTRACE_DETACH,cmd_list[i].p_id,NULL,SIGCONT);
                /*LOGGING STATE CHANGE*/
                log_state_change(cmd_list[i].p_id,cmd_list[i].r_status , PSTATE_RUNNING, 0);
                cmd_list[i].r_status=PSTATE_RUNNING;
                
                cmd_list[i].t_status='U';
                fprintf(stdout,"%d\t%d\t%c\t%s\t\t%s\n",cmd_list[i].deet_id,cmd_list[i].p_id,cmd_list[i].t_status,convert_status(cmd_list[i].r_status),cmd_list[i].command);

                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);

                return SKIP_NUM;
            }
            else {
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return NOMATCH_NUM;
            }

        return NOMATCH_NUM;
        }
         
        return NOMATCH_NUM;
    }

    else if(strcmp(word,"kill")==0){
        word = strtok(NULL," ");

        if(check_number(word)==1 && strtok(NULL," ")==NULL ){
            int d_id=atoi(word),i=0;

            Sigemptyset(&mask);
            Sigaddset(&mask,SIGCHLD);
            Sigaddset(&mask,SIGINT);
            Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

            while(cmd_list[i].deet_id!=d_id && i<MAXARGS){
                i++;
            }

            if(i==MAXARGS){
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return NOMATCH_NUM;
            }
            else if(cmd_list[i].r_status!=PSTATE_NONE && cmd_list[i].r_status!=PSTATE_KILLED  && cmd_list[i].r_status!=PSTATE_DEAD){
                Kill(cmd_list[i].p_id,SIGKILL);

                /*LOGGING STATE CHANGE*/
                log_state_change(cmd_list[i].p_id,cmd_list[i].r_status , PSTATE_KILLED, 0);
                cmd_list[i].r_status=PSTATE_KILLED;
                fprintf(stdout,"%d\t%d\t%c\t%s\t\t%s\n",cmd_list[i].deet_id,cmd_list[i].p_id,cmd_list[i].t_status,convert_status(cmd_list[i].r_status),cmd_list[i].command);


                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return SKIP_NUM;

            }
            else{
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return NOMATCH_NUM;
            }
        }
    }

    else if(strcmp(word,"wait")==0){
        word=strtok(NULL," ");
        int i=0;
        //is the second argument a deet id?
        if(check_number(word)==1){
            int d_id=atoi(word);

            Sigemptyset(&mask);
            Sigaddset(&mask,SIGCHLD);
            Sigaddset(&mask,SIGINT);
            Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

            while(cmd_list[i].deet_id!=d_id && i<MAXARGS){
                i++;
            }

            Sigprocmask(SIG_SETMASK,&prev_mask,NULL);

        }
        else{
            return NOMATCH_NUM;
        }

        word=strtok(NULL," ");

        //wait for the process to be dead
        if(word==NULL){


        Sigemptyset(&mask);
        Sigaddset(&mask,SIGCHLD);
        Sigaddset(&mask,SIGINT);
        Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

        while(cmd_list[i].r_status!=PSTATE_DEAD){
            Sigsuspend(&prev_mask);
        }

        Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
        return SKIP_NUM;

        }
        else if((strcmp(word,"running")==0 || strcmp(word,"stopping")==0  || strcmp(word,"stopped")==0 || strcmp(word,"continuing")==0 || strcmp(word,"killed")==0 || strcmp(word,"dead")==0) && strtok(NULL," ")==NULL){
        
            Sigemptyset(&mask);
            Sigaddset(&mask,SIGCHLD);
            Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

            if(strcmp(word,"running")==0){

                while(cmd_list[i].r_status!=PSTATE_RUNNING){
                Sigsuspend(&prev_mask);
                }
            }

            else if(strcmp(word,"stopping")==0){

                while(cmd_list[i].r_status!=PSTATE_STOPPING){
                Sigsuspend(&prev_mask);
                }
            }

            else if(strcmp(word,"stopped")==0){

                while(cmd_list[i].r_status!=PSTATE_STOPPED){
                Sigsuspend(&prev_mask);
                }
            }


            else if(strcmp(word,"continuing")==0){

                while(cmd_list[i].r_status!=PSTATE_CONTINUING){
                Sigsuspend(&prev_mask);
                }
            }


            else if(strcmp(word,"killed")==0){

                while(cmd_list[i].r_status!=PSTATE_KILLED){
                Sigsuspend(&prev_mask);
                }
            }


            else if(strcmp(word,"dead")==0){

                while(cmd_list[i].r_status!=PSTATE_DEAD){
                Sigsuspend(&prev_mask);
                }
            }

            Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
            return SKIP_NUM;

        }
        else{
            return NOMATCH_NUM;
        }


    }


    else if(strcmp(word,"quit")==0 && strtok(NULL," ")==NULL){
        //fprintf(stdout,"Reahced here");
        //fflush(stdout);
        for(int i=0;i<MAXARGS;i++){
             
            Sigemptyset(&mask);
            Sigaddset(&mask,SIGCHLD);
            Sigaddset(&mask,SIGINT);
            Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

            if(cmd_list[i].r_status!=PSTATE_DEAD && cmd_list[i].r_status!=PSTATE_NONE){
                //fprintf(stdout,"About to kill");
                //fflush(stdout);
                Kill(cmd_list[i].p_id,SIGKILL);

                /*LOGGING STATE CHANGE*/
                log_state_change(cmd_list[i].p_id,cmd_list[i].r_status , PSTATE_KILLED, 0);
                cmd_list[i].r_status=PSTATE_KILLED;
                fprintf(stdout,"%d\t%d\t%c\t%s\t\t%s\n",cmd_list[i].deet_id,cmd_list[i].p_id,cmd_list[i].t_status,convert_status(cmd_list[i].r_status),cmd_list[i].command);
                fflush(stdout);

            //unblock signals
            }
            //fprintf(stdout,"to pass\n");
            //fflush(stdout);
            while(cmd_list[i].r_status!=PSTATE_DEAD && cmd_list[i].r_status==PSTATE_KILLED){
                sigsuspend(&prev_mask);
            }
            //fprintf(stdout,"passed\n");
            //fflush(stdout);
            Sigprocmask(SIG_SETMASK,&prev_mask,NULL);

        }
        //fprintf(stdout,"want to exit");
        //fflush(stdout);
        /*
        Sigemptyset(&mask);
        Sigaddset(&mask,SIGCHLD);
        Sigprocmask(SIG_BLOCK,&mask,&prev_mask);



        while(finished_flag!=1){
            sigsuspend(&prev_mask);
         }

        Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
        */



        free(input);
        log_shutdown();
        exit(0);
        //Kill(getpid(),SIGKILL);

        return SKIP_NUM;
    }

    else if(strcmp(word,"peek")==0){

        Sigemptyset(&mask);
        Sigaddset(&mask,SIGCHLD);
        Sigaddset(&mask,SIGINT);
        Sigprocmask(SIG_BLOCK,&mask,&prev_mask);

        word=strtok(NULL," ");
        if(check_number(word)==1){

            int dee_id=atoi(word);
            int j=0;
            while(dee_id!=cmd_list[j].deet_id && j<MAXARGS){
                j++;
            }
            if(j==MAXARGS || (word=strtok(NULL," "))==NULL){
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return NOMATCH_NUM;
            }
            pid_t pid  = cmd_list[j].p_id;
            //unsigned long long address;

            //address=strtoull(word,NULL,16);
            void *ptr;
            sscanf(word,"%p",&ptr);
            word=strtok(NULL," ");


            if(word==NULL){
            fprintf(stdout,"%p\t%016x\n",ptr,(unsigned int)ptrace(PTRACE_PEEKDATA,pid,ptr,NULL));
            fflush(stdout);

            Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
            return SKIP_NUM;
            }

            else if(check_number(word)==1 && strtok(NULL," ")==NULL){
                int iter=atoi(word);

                for(int i=0;i<iter;i++){
                    
                    fprintf(stdout,"%p\t%016x\n",ptr,(unsigned int)ptrace(PTRACE_PEEKDATA,pid,ptr,NULL));
                    fflush(stdout);
                    ptr=(void*)((char*)ptr+64);
                }


                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return SKIP_NUM;
            }



        Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
        }
        else 
            return NOMATCH_NUM;
    }

    else if(strcmp(word,"poke")==0){
        word=strtok(NULL," ");

        if(check_number(word)==1){

            int dee_id=atoi(word);
            int j=0;
            while(dee_id!=cmd_list[j].deet_id && j<MAXARGS){
                j++;
            }

            if(j==MAXARGS || (word=strtok(NULL," "))==NULL){
                Sigprocmask(SIG_SETMASK,&prev_mask,NULL);
                return NOMATCH_NUM;
            }
            void *ptr;
            sscanf(word,"%p",&ptr);
            pid_t pid  = cmd_list[j].p_id;
            word=strtok(NULL," ");
            //unsigned long long address=strtoull(word,NULL,16);
            //void *ptr=(void*)address;
            int val=atoi(word);




            ptrace(PTRACE_POKEDATA,pid,ptr,val);
 
            return SKIP_NUM;
        }
        else 
            return NOMATCH_NUM;
    }


return NOMATCH_NUM;
}

void help_print(void){
    fprintf(stdout,"Available commands:\n");
    fprintf(stdout,"help -- Print this help message\n");
    fprintf(stdout,"quit (<=0 args) -- Quit the program\n");
    fprintf(stdout,"show (<=1 args) -- Show process info\n");
    fprintf(stdout,"run (>=1 args) -- Start a process\n");
    fprintf(stdout,"stop (1 args) -- Stop a running process\n");
    fprintf(stdout,"cont (1 args) -- Continue a stopped process\n");
    fprintf(stdout,"release (1 args) -- Stop tracing a process, allowing it to continue normally\n");
    fprintf(stdout,"wait (1-2 args) -- Wait for a process to enter a specified state\n");
    fprintf(stdout,"kill (1 args) -- Forcibly terminate a process\n");
    fprintf(stdout,"peek (2-3 args) -- Read from the address space of a traced process\n");
    fprintf(stdout,"poke (3 args) -- Write to the address space of a traced process\n");
    fprintf(stdout,"bt (1 args) -- Show a stack trace for a traced process\n");
    fflush(stdout);
}


void gettoken(char *input,char **argv){
    int argc=0;
    //fprintf(stdout,"%s",input);
    char *token=strtok(input," ");

    while(token!=NULL && argc<MAXARGS){
        
        argv[argc]=token;
        //fprintf(stdout,"%d : %s\n",argc,argv[argc]);
        argc++;
        token=strtok(NULL," \t\n");

    }
    //fprintf(stdout,"%d\n",argc);
    argv[argc]=NULL;

}

char *strip_command_line(char *sentence){
    char *str_ptr=sentence;
    while(*str_ptr && *str_ptr==' ')
        str_ptr++;

    while(*str_ptr && *str_ptr!=' ')
        str_ptr++;
    
    return str_ptr;
}


int check_number(char *token){
    int num_flag=1;
    for(int i=0;token[i]!='\0';i++){
        if(!isdigit((unsigned char)token[i]) && token[i]!='-'){
            num_flag=0;
            break;
        }
    }

    return num_flag;
}

char *convert_status(int temp_status){
    if(temp_status==PSTATE_NONE)
        return " ";
    else if(temp_status==PSTATE_RUNNING)
        return "running";
    else if(temp_status==PSTATE_STOPPING)
        return "stopping";
    else if(temp_status==PSTATE_STOPPED)
        return "stopped";
    else if(temp_status==PSTATE_CONTINUING)
        return "continuing";
    else if(temp_status==PSTATE_KILLED)
        return "killed";
    else if(temp_status==PSTATE_DEAD)
        return "dead";
    else 
        return " ";
    
}




