

#ifndef FINAL_PROJECT_FIRSTPASS_H
#define FINAL_PROJECT_FIRSTPASS_H
#include "structs.h"
#include "community_funcs.h"
#include <stdio.h>
#include "constants.h"


void insertToLabelTable(struct label_table **head, char input_label[MAX], short row, enum label_type t);

short extract_registernum(char *s);

void checkLabel(char *myLabel, struct reservedWords *reserved, struct label_table *head, short *error_found_p, short line_num, short *checker);

short checkAddressingMode013(char *operand, struct reservedWords* reserved);

short isAddressingMode2(short *code_array, char *operand, struct reservedWords* reserved, short *IC_p, short *L);

short commandCategory(char *myCommand, short *code_array, short *IC_p, short *error_found_p, short line_num, struct table *commands);

short parseTwoOperands(char *string, char *src_operand, char *dest_operand, short *error_found_p, short *L, short line_num);

void commandProcess(char *myCommand, char *operands, short *code_array, short *IC_p, short *error_found_p, short *L, short line_num, struct table *commads, struct reservedWords *reserved, short *sp_array);

short takeFirstWord(char *line, char *first_word, char *remainder, short *error_found_p, short line_num);

void firstPass(char *file_name, short *code_array, short *data_array, short *error_found_p, short *DC_p, short *IC_p, struct label_table **head, short *sp_array);

int extract_num(char *s);

void trimFirstAndLast(char *s);

void stringProcess(char *string, short *code_array, short *error_found_p, short line_num, short *DC_p, short *L);

short getNumber(char *s);

void removeWhite(char *s);

void dataProcess (char *string, short *code_array, short *error_found_p, short *DC_p, short *L, short line_num);

void increaseDataLabels(struct label_table *head, short IC_p);

short charAllowed(char s);

short spaceBetweenNums(char *s);

void insertToDataArray(short *data_array, short row, short val);

void spaceLabelAndLine(char *line);

void replaceTabsWithSpaces(char *s);

void delete14and15(short *array, short length);
#endif
