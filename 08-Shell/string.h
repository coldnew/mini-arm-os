#ifndef __STRING_H__
#define __STRING_H__

int strcmp(char *s1, char *s2);
int strncmp(char *s1, char *s2, int n);
size_t strlen(const char* s);
char *strtok(char *str, const char* delim);

#endif /* __STRING_H__ */