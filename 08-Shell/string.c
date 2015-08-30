#include <stddef.h>
#include <stdint.h>
#include "string.h"

int strcmp(char *s1, char *s2)
{
        while (*s1 != '\0' && *s1 == *s2) {
                s1++;
                s2++;
        }
        return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}

int strncmp(char *s1, char *s2, int n)
{
        if (n == 0) return 0;

        while (n-- != 0 && *s1 == *s2) {
                if (n == 0 || *s1 == '\0') break;
                s1++;
                s2++;
        }

        return (*(unsigned char *) s1) - (*(unsigned char *) s2);
}

size_t strlen(const char* s)
{
        size_t result = 0;
        while (*s++) ++result;
        return result;
}

char* strtok(char *str, const char* delim)
{
        static char* bp;

        if(str != NULL) bp = str;

        if(bp[0] == '\0') return NULL;

        char *ret = bp, *b;
        const char *d;

        for(b = bp; *b !='\0'; b++) {
                for(d = delim; *d != '\0'; d++) {
                        if(*b == *d) {
                                *b = '\0';
                                bp = b+1;

                                // skip the beginning delimiters
                                if(b == ret) {
                                        ret++;
                                        continue;
                                }
                                return ret;
                        }
                }
        }

        return ret;
}