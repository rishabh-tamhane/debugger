#ifndef __EVALUATION__
#define __EVALUATION__


int evaluate(char *cmdline);
void help_print(void);
void gettoken(char *input,char **argv);
char *strip_command_line(char *sentence);
int check_number(char *token);
char *convert_status(int temp_status);
#endif