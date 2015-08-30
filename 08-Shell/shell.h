#ifndef __SHELL_H__
#define __SHELL_H__

#define SHELL_MAX_ARGS 5

#define SHELL_MAX_ARG_LEN       15

extern char shell_args[SHELL_MAX_ARGS][SHELL_MAX_ARG_LEN];

int shell_arg_parser(char *cmd_line, int len, char (*args)[SHELL_MAX_ARG_LEN]);

#endif /* __SHELL_H__ */