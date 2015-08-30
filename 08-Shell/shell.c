#include <stdlib.h>
#include <string.h>

#include "shell.h"

char shell_args[SHELL_MAX_ARGS][SHELL_MAX_ARG_LEN];

int shell_arg_parser(char *cmd_line, int len, char (*args)[SHELL_MAX_ARG_LEN])
{
        int i, j;
        int spos = -1, argc = 0;

        for(i = 0; i < len; i++) {
                // to many arguments
                if(argc > SHELL_MAX_ARGS) return -1;

                if(cmd_line[i] == ' ' || i == len - 1) {
                        // catch last argument ...
                        if(i == len - 1) i++;

                        // argument value to long
                        if(i - spos > SHELL_MAX_ARG_LEN) return -2;

                        for(j = 0; j < i - spos - 1; j++) {
                                args[argc][j] = cmd_line[spos + 1 + j];
                        }
                        args[argc++][j] = '\0';
                        spos = i;
                }
        }

        return argc;
}
