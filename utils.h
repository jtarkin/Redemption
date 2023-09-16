/*
 * utility functions
 */
#ifndef UTILS_H
#define UTILS_H

bool str_cmp(const char *astr, const char *bstr);
bool str_prefix(const char *astr, const char *bstr);
bool str_infix(const char *astr, const char *bstr);
bool str_suffix(const char *astr, const char *bstr);
char *capitalize(const char *str);
char *cap_str(const char *str);
void sprintf_cat(char *target, const char *txt, ...);
char *capitalizeWords(const char *str);
void bug(const char *str, ...);
void log_string(const char *str, ...);
void lowercase(char *target, const char *original);
int str_len(const char *string);
char * long_crypt(char *pwd, char *salt); // Added to fix create_password crash on Ubuntu [by Aql, August 2018]

extern FILE *fpArea;
extern char strArea[MAX_INPUT_LENGTH];

#endif
