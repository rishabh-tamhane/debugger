#ifndef __ERROR_PROOF__
#define __ERROR_PROOF__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <ctype.h>

#define MAXARGS 128
#define	MAXLINE	 8192 



#define NOMATCH_NUM   1
#define HELP_NUM    2
#define QUIT_NUM    3
#define RUN_NUM    4
#define SHOW_NUM  5
#define CONT_NUM 6

#define SKIP_NUM 40





/* Our own error-handling functions */
void unix_error(char *msg);
void app_error(char *msg);

/* Process control wrappers */
pid_t Fork(void);

//extern cmd_box commands[MAXARGS];
//extern cmd_box *cmd_list; 


//SIO Functions
ssize_t sio_puts(char s[]);
static size_t sio_strlen(char s[]);
void sio_error(char s[]);
ssize_t Sio_puts(char s[]);

ssize_t sio_putl(long v);
ssize_t Sio_putl(long v);
static void sio_ltoa(long v, char s[], int b) ;
static void sio_reverse(char s[]);

//Signal Handler Functions
void sigchild_handler(int sig);




void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
void Sigemptyset(sigset_t *set);
void Sigfillset(sigset_t *set);
void Sigaddset(sigset_t *set, int signum);
void Sigdelset(sigset_t *set, int signum);
int Sigismember(const sigset_t *set, int signum);
int Sigsuspend(const sigset_t *set);

void Kill(pid_t pid, int signum) ;


void sigint_handler(int sig);

extern char *input;
#endif /* __ERROR_PROOF__ */
/* $end error_proof.h */