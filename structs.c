
#include "structs.h"
#include "constants.h"
#include <string.h>


/* For encapsulation purposes, the structs for the data needed for the program are stored here, and are available to the functions via the getters written below */

/*all the words which are reserved and can't be used as label or macro names*/
struct reservedWords reservedTable[] = {
        {"mov", 1},
        {"cmp", 1},
        {"add", 1},
        {"sub", 1},
        {"not", 1},
        {"clr", 1},
        {"lea", 1},
        {"inc", 1},
        {"dec", 1},
        {"jmp", 1},
        {"bne", 1},
        {"red", 1},
        {"prn", 1},
        {"jsr", 1},
        {"rts", 1},
        {"stop", 1},
        {".extern", 1},
        {".data", 1},
        {".entry", 1},
        {".string", 1},
        {"r0", 1},
        {"r1", 1},
        {"r2", 1},
        {"r3", 1},
        {"r4", 1},
        {"r5", 1},
        {"r6", 1},
        {"r7", 1}
};
/*all the commands and their respective category types.*/
struct table commandsTable[] = {
        {"mov", 1},
        {"cmp", 1},
        {"add", 1},
        {"sub", 1},
        {"not", 2},
        {"clr", 2},
        {"lea", 1},
        {"inc", 2},
        {"dec", 2},
        {"jmp", 2},
        {"bne", 2},
        {"red", 2},
        {"prn", 2},
        {"jsr", 2},
        {"rts", 3},
        {"stop", 3},
        {"error", 0}
};
/*returns a pointer to beginning of the reserved table*/
struct reservedWords *get_reserved(void) {
    return reservedTable;
}
/*returns a pointer to the beginning of the command table*/
struct table *get_commands(void) {
    return commandsTable;
}

/* receives a string and checks if its one of our reserved words */
short isReserved(char *str, struct reservedWords* reserved)
{
    short i;
    for (i=0; i<RESERVEDNUM; i++)
    {
        if (strcmp(str, reserved[i].resword) == 0)
            return reserved[i].ans;
    }
    return 0;
}
