

#ifndef FINAL_PROJECT_STRUCTS_H
#define FINAL_PROJECT_STRUCTS_H
#include "constants.h"

enum label_type
{
    code,
    external,
    stringtype,
    datatype,
    no_label_type
};

enum word
{
    data,
    string,
    entry,
    ext,
    label,
    command,
    error,
    nullword
};
struct table
{
    char name[NAMELENGTH];
    short category;
};

struct reservedWords {
    char resword[NAMELENGTH];
    short ans;
};

struct label_table
{
    char label[MAX];
    short row;
    enum label_type type;
    struct label_table *next;
};

short findInLabelTable(struct label_table *head, char cmp[MAX]);
struct table *get_commands(void);
short isReserved(char *str, struct reservedWords* reserved);
struct reservedWords *get_reserved(void);



#endif
