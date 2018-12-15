#ifndef __PARSER_H
#define __PARSER_H

#define BUFF_SIZE 300
#define ARGS_COUNT 30

enum GetSongInfoResult
{
   NO_METADATA, TITLE_ONLY, ARTIST_ONLY, FULL_METADATA
};

void Parse_init();
uint8_t getSongInfo();
void Parse( uint8_t);

#endif
