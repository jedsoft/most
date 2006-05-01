#ifndef _DAVIS_SEARCH_H_
# define _DAVIS_SEARCH_H_
# include <stdio.h>
# include <string.h>

extern int Most_Case_Sensitive;
extern int Most_Search_Dir;
#define MOST_SEARCH_BUF_LEN	256
extern char Most_Search_Str[MOST_SEARCH_BUF_LEN];
extern int most_search(unsigned char *, int, int *);
#endif

