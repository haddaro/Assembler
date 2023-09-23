

#include "firstPass.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

/* this function reads the text in *file and does the following:
1. Checks for all kinds of syntax errors (wrong words, wrong operands, superfluous charachters etc.).
2. Makes sure that a label is not used more than once.
3. Creates a label table with all the labels decalerd in line beginnings and after ".extern".
4. Encodes the opcode of every command into the code array.
5. Encodes the addressing modes 0,1, 2 or 3 to the code array.
6. If a command has operands of addressing mode 0 or 3, it encodes the numbers / register-numbers to the code array. If a command has operands of addressing mode 2, then the addressing mode of the parameters are encoded into the array, as well as numbers and register-numbers (the coding of label addresses will be dealt with in the second pass).
7. Encodes the numbers after ".data" to the data array.
8. Encodes the charachters of the strings after .string to the data array.
9. Updates an array that directs the second pass into its relevant lines: (-1) for ".entry" lines and (1) for lines with addressing modes 1 or 2 in the commands.
10. If an error is found - it lights up an error flag so the second pass will work in error finding mode.
*/

void firstPass(char* file_name, short *code_array, short *data_array, short *error_found_p, short *DC_p, short *IC_p, struct label_table **head, short *sp_array)
{

    char line [MAX];
    char first_word[MAX];
    char myLabel[MAX];
    char remainder[MAX];
    char operands[MAX];
    char myCommand[MAX];
    short line_num = 0;
    short label_found;
    short L;
    short checker = 0;
    enum word word_status;
    struct reservedWords *reserved;
    struct table *commands;
    FILE *file;
    file = my_fopen(file_name,"am","r");
    if(file==NULL){
        printf("\nfatal error in firstPass- there was an error opening a file!");
        exit(0);
    }

    reserved = get_reserved();
    commands = get_commands();

    while (fgets(line, MAX, file) != NULL) /*we've read a line*/
    {
        L = 0;
        label_found = 0;
        word_status = nullword;
        myCommand[0] = '\0';
        myLabel[0] = '\0';
        operands[0] = '\0';
        line_num++;

        replaceTabsWithSpaces(line); /* to help the coming functions */

        if (strlen(line) > MAXLINEALLOWED)
        {
            fprintf(stdout, "Error in line %d: line is too long\n", line_num);
            *error_found_p = 1;
            continue;
        }

        /*get rid of white spaces before and after the text*/
        trimWhite(line);
        if (line[0] == ';' || line[0] == '\0' || line[0] == '\n')
            continue; /* comment line or empty line - we don't read it */

        spaceLabelAndLine(line); /* solves the end case of no space between a label's : and the rest of the line*/
        /* work separately on the first word and on all the rest: */
        if (takeFirstWord(line, first_word, remainder, error_found_p, line_num) == 0)
            continue;

        /*check what the first word is: */
        word_status = word_parser(first_word, myLabel, myCommand);

        if (word_status == label)
        {
            label_found = 1; /* the label is meanwhile stored in myLabel, lets check if its ok */
            checkLabel(myLabel, reserved, *head, error_found_p, line_num, &checker);
            if (takeFirstWord(remainder, first_word, operands, error_found_p, line_num) == 0)
                continue;
            /*start parsing the line after the label*/
            strcpy(remainder, operands);
            word_status = word_parser(first_word, myLabel, myCommand);
        }

        switch (word_status)
        {
            case nullword:
                fprintf(stdout, "Error in line %d: Parsing error occurred\n", line_num);
                *error_found_p = 1;
                break;
            case error:
                fprintf(stdout, "Error in line %d: Ilegal word: %s\n", line_num, first_word);
                *error_found_p = 1;
                break;
            case label:
                if (label_found == 1) /* we checked for labels before, no other label should appear */
                {
                    *error_found_p = 1;
                    fprintf(stdout, "Error in line %d: two labels one after the other.\n", line_num);
                }
                break;
            case data:
                if (label_found == 1)
                {
                    if (*error_found_p != 1)
                        insertToLabelTable(head, myLabel, (*DC_p)+100, datatype); /* we insert the label to the linked list as data */
                }
                dataProcess (remainder, data_array, error_found_p, DC_p, &L, line_num);
                *DC_p += L+1;
                break;
            case string:
                if (label_found == 1)
                {
                    if (*error_found_p != 1)
                        insertToLabelTable(head, myLabel, (*DC_p)+100, stringtype); /* we insert the label to the linked list as string */
                }

                /*remainder is supposed to be a a string*/
                trimWhite(remainder);
                /* call the function that check and encodes strings and updates L*/
                stringProcess(remainder, data_array, error_found_p, line_num, DC_p, &L);
                *DC_p += L+1;
                break;
            case entry:
                if(isLegalLabel(remainder))
                    sp_array[line_num-1] = -1;
                else
                    fprintf(stdout,"Warning for line %d: Label after .entry line is illegal.\n",line_num);
                break;
            case ext:
                if (label_found == 1)
                {
                    fprintf(stdout, "Warning for line %d: declaration of label at the beginning on an .extern line is meaningless.\n", line_num);
                }
                trimWhite(remainder);
                checkLabel(remainder, reserved, *head, error_found_p, line_num, &checker);
                if (*error_found_p != 1)
                    insertToLabelTable(head, remainder, -1, external);
                break;
            case command: /* the suspected name of the command is stored in myCommand, and the operands would be in remainder */

                if (label_found == 1)
                {
                    if (*error_found_p != 1)
                        insertToLabelTable(head, myLabel, (*IC_p)+100, code); /* we insert the label to the linked list as type code */
                }
                commandProcess(myCommand, remainder, code_array, IC_p, error_found_p, &L, line_num, commands, reserved, sp_array);
                *IC_p +=L+1;


        }/*end of switch-case*/

    }/*end of while*/

    /*Increase the row of every data/string label by the value of IC*/
    increaseDataLabels(*head, *IC_p);
    delete14and15(code_array, MAXCODELINES); /* since we have 16 bits in every short, we want to get rid of 11 of negative numbers in the bits we don't need */
    delete14and15(code_array, MAXCODELINES);
fclose(file);
} /*end of first pass */

void spaceLabelAndLine(char *line) {

    char *ptr;
    ptr = strchr(line, ':');
    if (ptr!=NULL && !isspace((*ptr) + 1))
    {
        memmove(ptr+2, ptr+1, strlen(ptr));
        *(ptr+1) = ' ';
    }

}

/* Receives a string that consists of at least 2 words, and separate the first word from the rest. Returns 0 if an error was found */
short takeFirstWord(char *line, char *first_word, char *remainder, short *error_found_p, short line_num)
{
    char *token;
    char *space;
    char *sharp;
    char *comma;

    /*check first char*/
    if (!isalpha(line[0]) && line[0] != '.')
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: words cannot begin with the char %c\n", line_num, line[0]);
        first_word[0] = '\0';
        remainder[0] = '\0';
        return 0;;
    }
    space = strchr(line, ' ');
    sharp = strchr(line, '#');
    comma = strchr(line, ',');
    if (space == NULL && sharp == NULL && comma == NULL && strcmp(line, "rts") && strcmp(line, "stop")) /* only two words are allowed to appear alone */
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: no space between words\n", line_num);
        first_word[0] = '\0';
        remainder[0] = '\0';
        return 0;
    }
    if (space == NULL) /* only one word, which is allowed */
    {
        strcpy(first_word, line);
        remainder[0] = '\0';
        return 1;
    }
    token = strtok(line, " \t");
    strcpy(first_word, token);
    token = strtok(NULL, "");
    strcpy(remainder, token);
    trimWhite(remainder);
    return 1;
}




/* receives a string and checks if its a legal label that is not a reserved word and has not been entered already*/
void checkLabel(char *myLabel, struct reservedWords *reserved, struct label_table *head, short *error_found_p, short line_num, short *checker)
{
    *checker = 0;
    if (!isLegalLabel(myLabel))
    {
        fprintf(stdout, "Error in line %d: %s cannot be a label\n", line_num, myLabel);
        *error_found_p = 1;
        *checker = 1;
    }
    else if (isReserved(myLabel, reserved))
    {
        fprintf(stdout, "Error in line %d: %s is a reserved word\n", line_num, myLabel);
        *error_found_p = 1;
        *checker = 1;
    }
    else if (findInLabelTable(head, myLabel) != 0)
    {
        fprintf(stdout, "Error in line %d: The label %s is used more than once.\n", line_num, myLabel);
        *error_found_p = 1;
        *checker = 1;
    }
}


/* insertion into linked list */
void insertToLabelTable(struct label_table **head, char input_label[MAX], short row, enum label_type t)
{
    struct label_table *newNode = (struct label_table *)calloc(1,sizeof(struct label_table));
    strcpy(newNode->label, input_label);
    newNode->row = row;
    newNode->type = t;
    newNode->next = NULL;
    if (*head == NULL)
    {
        *head = newNode;
    }
    else
    {
        newNode->next = *head;
        *head = newNode;
    }
}

/* Linear search in linked list. Returns 0 if cmp is not found in the list, and its row if it is */
short findInLabelTable(struct label_table *head, char cmp[MAX])
{
    struct label_table *p;
    p = head;
    while (p != NULL)
    {
        if (strcmp(p->label, cmp) == 0)
            return p->row;
        else
            p = p->next;
    }
    return 0;
}

/* the function that initiates everything that happens after finding a word suspected as a command*/
void commandProcess(char *myCommand, char *operands, short *code_array, short *IC_p, short *error_found_p, short *L, short line_num, struct table *commands, struct reservedWords *reserved, short *sp_array)
{
/*myCommand has something that we suspect to be a command, and operands has the rest of the line*/

    short category;
    short addressing_src;
    short addressing_dest;
    char src_operand[MAX];
    char dest_operand[MAX];
    char *ptr = NULL;

/* first, the function commandCategory will check if its a valid command. If it is, it will encode the number of its command to the code array and return the category of operands). If it isn't, it will return 0 */

    category = commandCategory(myCommand, code_array, IC_p, error_found_p, line_num, commands);
    if (category == 3 && operands[0] != '\0')
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: command %s should not take parameters.\n", line_num, myCommand);
        return;
    }
    if (category == 1) /*we expect two legal operands separated by a comma*/
    {
        /*parseTwo operands will check if there are two operands separated by a comma, and if so, will store them at src_operand and dest_operand. If not it will throw en error */
        if (parseTwoOperands(operands, src_operand, dest_operand, error_found_p, L, line_num) == 0)
            return;
        addressing_src = checkAddressingMode013(src_operand, reserved);
        addressing_dest = checkAddressingMode013(dest_operand, reserved);
        if (addressing_src == 1 || addressing_dest == 1) /* this helps the second pass know where to look */
            sp_array[line_num-1] = *IC_p;
        if ((addressing_src == -1) || (addressing_dest == -1) || (addressing_dest == 0 && strcmp(myCommand, "cmp") != 0) || (addressing_src != 1 && strcmp(myCommand, "lea") == 0))
        {
            *error_found_p = 1;
            fprintf(stdout, "Error in line %d: wrong addressing mode for command %s\n", line_num, myCommand);
            return;
        }
        /*now we can code the addressing modes to the array*/
        if (*error_found_p != 1)
        {
            insertToArray(code_array, *IC_p, addressing_src, SOURCEOPERAND);
            insertToArray(code_array, *IC_p, addressing_dest, DESTOPERAND);
        }

        if (addressing_src == 3 && addressing_dest == 3)
        {
            if (*error_found_p != 1)
            {
                *L = 1; /*both are registers so we need one word */
                /* encode register numbers to the same 'word' */
                insertToArray(code_array, (*IC_p)+ *L, extract_registernum(src_operand), SOURCEREGISTER);
                insertToArray(code_array, *IC_p + *L, extract_registernum(dest_operand), DESTREGISTER);

            }

            return;
        }
        /* encode registers and numbers */
        if (*error_found_p != 1)
        {
            if (addressing_src == 3)
                insertToArray(code_array, (*IC_p) + 1, extract_registernum(src_operand), SOURCEREGISTER);
            if (addressing_dest == 3)
                insertToArray(code_array, (*IC_p) + 2, extract_registernum(dest_operand), DESTREGISTER);
            if (addressing_src == 0)
                insertToArray(code_array, (*IC_p) + 1, extract_num(src_operand), OPERAND);
            if (addressing_dest == 0)
                insertToArray(code_array, (*IC_p) + 2, extract_num(src_operand), OPERAND);
            *L = 2;
        }
        return;
    }
    if (category == 2) /*we expect one legal operand*/
    {
        /*check for distinct words*/
        ptr = strchr(operands, ' ');
        if (ptr !=NULL)
        {
            *error_found_p = 1;
            fprintf(stdout, "Error in line %d: wrong operand syntax.\n", line_num);
            return;
        }
        addressing_dest = checkAddressingMode013(operands, reserved);
        if (addressing_dest == 1)
            sp_array[line_num-1] = *IC_p;
        if (addressing_dest == 0 && strcmp(myCommand, "prn") != 0)
        {
            *error_found_p = 1;
            fprintf(stdout, "Error in line %d: wrong addressing mode for command %s\n", line_num, myCommand);
            return;
        }
        if (addressing_dest == 0) /* this is prn */
        {
            *L=1;
            insertToArray(code_array, (*IC_p) + *L, extract_num(operands), OPERAND);

        }
        if (addressing_dest !=-1)
        {
            if (*error_found_p != 1)
            {
                *L = 1;
                /*encode addressing mode, and register / number */
                insertToArray(code_array, *IC_p, addressing_dest, DESTOPERAND);
                if (addressing_dest == 3)
                {
                    insertToArray(code_array, (*IC_p) + *L, extract_registernum(dest_operand), OPERAND);
                }
                if (addressing_dest == 0)
                {
                    insertToArray(code_array, (*IC_p) + *L, extract_num(dest_operand), OPERAND);
                }
            }
        }
        if (addressing_dest == -1)
        {
            /*chek if it returned -1 because of a problem */
            if (!isalpha(operands[0]))
            {
                *error_found_p = 1;
                fprintf(stdout, "Error in line %d: %s is an invalid operand\n", line_num, operands);
                return;
            }
            /*it can be addressing mode 2, but not for every function */
            if (strcmp(myCommand, "jmp") != 0 && strcmp(myCommand, "bne") != 0 && strcmp(myCommand, "jsr") != 0)
            {
                *error_found_p = 1;
                fprintf(stdout, "Error in line %d: wrong addressing mode for command %s\n", line_num, myCommand);
                return;
            }
            if (!isAddressingMode2(code_array, operands, reserved, IC_p, L)) /*if it is indeed addressing mode 2, the function updates the L according to the parameters and encodes everything except for label addresses. */
            {

                *error_found_p = 1;
                fprintf(stdout, "Error in line %d: wrong addressing mode for command %s\n", line_num, myCommand);
            }
            else
                sp_array[line_num-1] = *IC_p;
        }/*end of if (-1)*/
    }/*end of if category is 2*/
}/*end of commandProcess */

/* Receives a string and trims its first and last charasters. Used to remove "..." from a string.*/
void trimFirstAndLast(char *s)
{
    short len = strlen(s);
    if (len > 0)
    {
        memmove(s, s+1, len);
        if (len > 1)
            s[len-2] = '\0';
        else
            s[0] = '\0';
    }
}

/* handles strings: checks if the string is valid and if so, encodes in character by character in the data arra */
void stringProcess(char *string, short *data_array, short *error_found_p, short line_num, short *DC_p, short *L)
{
    short i;
    if (string[0] != '\"' || string[strlen(string)-1] !='\"')
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: string not inside double quotes\n", line_num);
        return;
    }
    /* get rid of quotes: */
    trimFirstAndLast(string);
    if (string[0] == '\0')
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: invalid string\n", line_num);
        return;
    }
    for (i = 0; i<strlen(string); i++)
    {
        if (string[i] < 0 || string[i] > 127)
        {
            *error_found_p = 1;
            fprintf(stdout, "Error in line %d: string should only contain ascii characters\n", line_num);
            return;
        }
        insertToDataArray(data_array, *(DC_p) + i, string[i]);
    }
    *L = i; /* i is now one more than the number of chars, so the i-th word after the DC_p we received will remain empty*/
}

/*Receives a string in the form of r1, r2..., that has already been checked, ane returns the number of the register*/
short extract_registernum(char *s)
{
    return (s[1] - '0');
}

/* Receives a pointer to a string as its first parameter. If the string is a valid command, it encodes its number's binary representation to the code array, and returns the category of the command (category determines how many operands). If it isn't, it prints an error and returns 0.*/
short commandCategory(char *myCommand, short *code_array, short *IC_p, short *error_found_p, short line_num, struct table *commands)
{
    short i;
    short category;
    for (i = 0; i < COMMANDNUM; i++)
    {
        if (strcmp(myCommand, commands[i].name) == 0)
        {
            insertToArray(code_array, *IC_p, i, OPCODE); /*the number of the command is now encoded to the code table */
            category = commands[i].category;
        }
    }
    if (category == 0)
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: command %s not found.\n", line_num, myCommand);
    }
    return category;
}

/*Receives a string and if if the string is 2 words separated by a comma, stores them. Else - throws an error and returns 0*/
short parseTwoOperands(char *string, char *src_operand, char *dest_operand, short *error_found_p, short *L, short line_num)
{
    char *token;
    short len;

    /* check if the first and last chars are alphanumeric or allowed */
    trimWhite(string);
    len = strlen(string) -1;

    if (!charAllowed(string[0]))
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: the character %c is not allowed in the beginning of %s\n", line_num, string[0], string);
        return 0;
    }
    if (!charAllowed(string[len]))
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: the character %c is not allowed at the end of %s\n", line_num, string[0], string);
        return 0;
    }


    token = strtok(string, ",");
    if (token == NULL)
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: too few parameters.\n", line_num);
        return 0;
    }
    strcpy(src_operand, token);
    token = strtok(NULL, ",");
    if (token == NULL)
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: too few parameters\n", line_num);
        return 0;
    }
    strcpy(dest_operand, token);
    token = strtok(NULL, ",");
    if (token != NULL)
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: too many operands\n", line_num);
        return 0;
    }
    /*now the first operand and the second operand are stored, but they can have white spaces at the beginning and the end so we should trim them*/
    trimWhite(src_operand);
    trimWhite(dest_operand);

    /*now check if they begin and end with legal chars */
    if (!charAllowed(src_operand[strlen(src_operand) - 1]) || !charAllowed(dest_operand[0]))
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: wrong operand syntax\n", line_num);
        return 0;
    }
    /*and check if dest_operands is more than one word*/
    token = strchr(dest_operand, ' ');
    if (token !=NULL)
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: too many parameters\n", line_num);
        return 0;
    }
    token = strchr(dest_operand, ',');
    if (token !=NULL)
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: too many parameters\n", line_num);
        return 0;
    }
    return 1;

}
/* checks for some chars that are not allowed */
short charAllowed(char s)
{
    if (!isdigit(s) && !isalpha(s) && s != '#' && s != '-' && s !='+')
        return 0;
    else return 1;
}




/* endoes val into the data array in the specified row */
void insertToDataArray(short *data_array, short row, short val)
{
    *(data_array + row) = val;
}


/*Receives an operand and returns its addressing mode except 2. Returns -1 if addressing mode is not 0, 1 or 3. */
short checkAddressingMode013(char *operand, struct reservedWords* reserved)
{
    char temp[MAX];
    if (operand[0] == '\0')
    {

        return -1;
    }
    if  (operand[0] == '#')
    {
        strcpy(temp, operand+1);
        if (isNumber(temp))
            return 0;
    }
    if (operand[0] == 'r' && operand[1] >= '0' && operand[1] <= '7' && strlen(operand) == 2)
    {
        return 3;
    }
    if (isLegalLabel(operand) == 1 && !isReserved(operand, reserved))
        return 1;
    return -1;
}

/* receives and operand cluster and checks if it fits addressing mode 2*/
short isAddressingMode2(short *code_array, char *operand, struct reservedWords* reserved, short *IC_p, short *L)
{
    char label[MAX];
    char param1[MAX];
    char param2[MAX];
    char *token;
    short param1_addressing;
    short param2_addressing;
    char *ptr;

    if (operand[strlen(operand)-1] != ')')
        return 0;
    ptr = strchr(operand, '(');
    if (ptr == NULL)
        return 0;
    ptr = strchr(operand, ',');
    if (ptr == NULL)
        return 0;
    /*now tokenize it to see if its a label followed by (...,...) */
    token = strtok(operand, "()");
    strcpy(label, token);
    if (!isLegalLabel(label))
        return 0;
    if (isReserved(label, reserved))
        return 0;
    token = strtok(NULL, ",");
    strcpy(param1, token);
    token = strtok(NULL, ",");
    strcpy(param2, token);
    token = strtok(NULL, ",");
    if (token != NULL)
        return 0;
    param2[strlen(param2)-1] = '\0'; /* delete the ) at the end */
    /*now check if the parameters inside the () are ok */
    param1_addressing = checkAddressingMode013(param1, reserved);
    param2_addressing = checkAddressingMode013(param2, reserved);
    if (param1_addressing == -1 || param2_addressing == -1)
        return 0;

    /*now we can encode the addressing modes of the function and parameters into the code array*/
    /* we have found that the destination addressing mode for the function is 2 */
    insertToArray(code_array, *IC_p, 2, DESTOPERAND);

    /* and we know the addressing modes of the parameters */
    insertToArray(code_array, *IC_p, param1_addressing, PARAM1);
    insertToArray(code_array, *IC_p, param2_addressing, PARAM2);

    /* and now for the calculation of L */

    if (param1_addressing == 3 && param2_addressing ==3)
    {
        *L = 2; /* both are registers so only one word */
        /* Leave one word empty and encode the registers */
        insertToArray(code_array, *IC_p +2, extract_registernum(param1), SOURCEREGISTER);
        insertToArray(code_array, *IC_p + 2, extract_registernum(param2), DESTREGISTER);
        return 1; /* it is addressing mode 2*/
    }
    /*if we got to this point then the parameters after the label are legal but not two registers so we need two words */
    *L = 3;

    /*encode the first parameter if we can */
    if (param1_addressing == 3)
        insertToArray(code_array, *IC_p +2, extract_registernum(param1), SOURCEREGISTER);
    else if (param1_addressing == 0)
        insertToArray(code_array, *IC_p +2, (short)extract_num(param1), OPERAND);

    /*and encode the second parameter if we can */
    if (param2_addressing == 3)
        insertToArray(code_array, *IC_p +3, extract_registernum(param2), DESTREGISTER);
    else if (param2_addressing == 0)
        insertToArray(code_array, *IC_p +3, (short)extract_num(param2), OPERAND);



    return 1;
}


/* checks if the string can be a legal label */


/*Receives a string in the form of #123 that has already been checked, ane returns the number after the sharp sign*/
int extract_num(char *s)
{
    short i;
    char *p;
    char num_str[MAX];
    i = 0;
    p = s + 1;

    while (*p != '\0')
        num_str[i++] = *p++;
    num_str[i] = '\0';

    return atoi(num_str);
}

/* receives a string which is a number that may have a - or + and returns the number */
short getNumber(char *s)
{
    short ans;
    if (s[0] == '-')
        ans = (short)(-1) * extract_num(s);
    else if (s[0] == '+')
        ans = (short)extract_num(s);
    else
        ans = (short)atoi(s);
    return ans;
}

/*handles data */
void dataProcess (char *string, short *data_array, short *error_found_p, short *DC_p, short *L, short line_num)
{
    char *token;
    short number;
    short i;

    i = 0;
    if (spaceBetweenNums(string))
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: invalid list after .data\n", line_num);
        return;
    }

    /* now we can safely remove spaces */
    removeWhite(string);

    if (string[strlen(string)-1] == ',')
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: data ends with a comma\n", line_num);
        return;
    }

    if(strstr(string, ",,") != NULL)
    {
        *error_found_p = 1;
        fprintf(stdout, "Error in line %d: two commas one after the other.\n", line_num);
        return;
    }

    if (isNumber(string)) /*there is only one number */
    {
        *L = 0;
        number = getNumber(string);
        insertToDataArray(data_array, *DC_p + *L, number);
        return;
    }

    /* tokenize based on comma */
    token = strtok(string, ",");
    while (token != NULL)
    {
        if (isNumber(token))
        {
            number = getNumber(token);
            insertToDataArray(data_array, *DC_p + i, number);
            i++;
        }
        else
        {
            *error_found_p = 1;
            fprintf(stdout, "Error in line %d: parameters for .data should be integers\n", line_num);
        }
        token = strtok(NULL, ",");

    }
    *L = i-1;

}

/* checks if the string contains spaces that are not between commas */
short spaceBetweenNums(char *s)
{
    short i;
    short lastNonWhite;
    trimWhite(s);
    lastNonWhite = s[0];
    for (i=1; i<strlen(s)-1; i++)
    {
        if (isspace(s[i]))
        {
            if (lastNonWhite != ',')
                return 1;
        }
        else
            lastNonWhite = s[i];
    }
    return 0;
}
/*removes every white space from the string */
void removeWhite(char *s)
{
    char *source;
    char *dest;
    source = s;
    dest = s;
    while (*source)
    {
        if (!isspace((unsigned char) *source)) /*we need to cast the char so isspace will behave */
            *dest++ = *source; /* copy the value in 'source' to 'dest' and then increment dest*/
        source++;
    }
    *dest = '\0';
}
/* Receives the value of IC_p and increases the row number of every data/string label by it */
void increaseDataLabels(struct label_table *head, short IC_p)
{
    struct label_table *p;
    p = head;
    while (p != NULL)
    {
        if (p->type == datatype || p->type == stringtype)
            p->row += IC_p;
        p = p->next;
    }
}



/* puts 0 in bits 14 and 15 os shorts */
void delete14and15(short *array, short length)
{
    short i;
    for (i = 0; i < length; i++)
        array[i] &= ~(3 << 14);
}

/* replaces tabs in a string with space */
void replaceTabsWithSpaces(char *s)
{
    short i;
    for (i = 0; s[i] != '\0'; i++)
    {
        if (s[i] == '\t')
            s[i] = ' ';
    }

}



















