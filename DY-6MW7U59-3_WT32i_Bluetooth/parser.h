#ifndef __PARSER_H
#define __PARSER_H

#define BUFF_SIZE 300
#define ARGS_COUNT 30

void Parse_init();
uint8_t getSongInfo();
void Parse(uint8_t);
uint8_t isParsing(void);

#endif
