#include <stm32f10x.h>

#include "parser.h"
#include "iwrap.h"
#include "display_handler.h"
#include "string.h"

uint16_t str_idx;
uint8_t buf[BUFF_SIZE];
uint8_t* argv[ARGS_COUNT];
uint8_t argc;
uint8_t text_flag;
uint8_t parsing = 0;

uint16_t _atoi(uint8_t*);
uint8_t isStrEqual(uint8_t*, uint8_t*);
int8_t findIndexOfToken(uint8_t*);

extern uint8_t displayStringBuffer[DISPLAY_STRING_SIZE];
extern uint8_t displayDataBuffer[DISPLAY_DATA_SIZE];
extern void resetDisplayState();
extern void ClearDisplayString();
extern enum PlaybackState playbackState;

struct SongInfo
{
   uint8_t* Title;
   uint8_t* Artist;
   uint8_t* Album;
} songInfo;

uint16_t unicodeToChar(uint16_t w_char);

uint16_t _atoi(uint8_t* str)
{
   uint16_t res = 0;
   for (uint8_t i = 0; str[i] != '\0'; ++i)
      res = res * 10 + str[i] - '0';
   return res;
}

int8_t findIndexOfToken(uint8_t* token)
{
   for (int i = 0; i < argc; i++)
   {
      if (isStrEqual(argv[i], token))
         return i;
   }
   return -1;
}

uint8_t getSongInfo()
{
   int8_t artist_idx = findIndexOfToken((uint8_t*) "ARTIST");
   //int8_t album_idx = findIndexOfToken((uint8_t*) "ALBUM");
   int8_t title_idx = findIndexOfToken((uint8_t*) "TITLE");

   if (artist_idx == -1 && title_idx == -1)
      return NO_METADATA;

   if (_atoi(argv[artist_idx + 1]) == 0 && _atoi(argv[title_idx + 1]) == 0)
      return NO_METADATA;

   songInfo.Artist =
         (artist_idx == -1 || _atoi(argv[artist_idx + 1]) == 0) ?
               0x00 : argv[artist_idx + 2];

   songInfo.Title =
         (title_idx == -1 || _atoi(argv[title_idx + 1]) == 0) ?
               0x00 : argv[title_idx + 2];
   if (!songInfo.Artist)
      return TITLE_ONLY;
   if (!songInfo.Title)
      return ARTIST_ONLY;
   return FULL_METADATA;
}

void HandleParseData()
{
   uint16_t cnt = 0;
   uint8_t idx = 0;
   uint16_t unicode_char;

   if (memcmp(argv[0], (uint8_t*) "AVRCP", 5) == 0)
   {
      if (isStrEqual(argv[2], (uint8_t*) "GET_ELEMENT_ATTRIBUTES_RSP"))
      {

         if (getSongInfo() == NO_METADATA)
         {
            ClearDisplayString();
            //resetDisplayState();
            return;
         }

         cnt = 0;
         idx = 0;
         if (songInfo.Title)
         {
            do
            {
               if (songInfo.Title[idx] & 0x80 && songInfo.Title[idx + 1] & 0x80)
               { // unicode mask
                  unicode_char = ((uint16_t) songInfo.Title[idx] << 8)
                        | songInfo.Title[idx + 1];

                  unicode_char = unicodeToChar(unicode_char);
                  if (unicode_char & 0xFF00)
                  {
                     displayDataBuffer[cnt] = (uint8_t)(unicode_char >> 8);
                     cnt++;
                     displayDataBuffer[cnt] = (uint8_t) unicode_char;
                  }
                  else
                     displayDataBuffer[cnt] = (uint8_t) unicode_char;
                  idx++;
               }
               else
                  displayDataBuffer[cnt] = songInfo.Title[idx];
            } while (songInfo.Title[idx++] != 0 && cnt++ < DISPLAY_DATA_SIZE);

            if (songInfo.Artist)
            {
               displayDataBuffer[cnt++] = ' ';
               displayDataBuffer[cnt++] = '-';
               displayDataBuffer[cnt++] = ' ';
            }
         }

         idx = 0;
         if (songInfo.Artist)
         {
            do
            {
               if (songInfo.Artist[idx] & 0x80
                     && songInfo.Artist[idx + 1] & 0x80)
               { // unicode mask
                  unicode_char = ((uint16_t) songInfo.Artist[idx] << 8)
                        | songInfo.Artist[idx + 1];

                  unicode_char = unicodeToChar(unicode_char);
                  if (unicode_char & 0xFF00)
                  {
                     displayDataBuffer[cnt] = (uint8_t)(unicode_char >> 8);
                     cnt++;
                     displayDataBuffer[cnt] = (uint8_t) unicode_char;
                  }
                  else
                     displayDataBuffer[cnt] = (uint8_t) unicode_char;
                  idx++;
               }
               else
                  displayDataBuffer[cnt] = songInfo.Artist[idx];
            } while (songInfo.Artist[idx++] != 0 && cnt++ < DISPLAY_DATA_SIZE);
         }

         idx = 0;
         cnt++;
         for (int i = cnt; i < DISPLAY_DATA_SIZE; i++)
         {
            displayDataBuffer[i] = 0;
         }

         idx = 0;
         resetDisplayState();

         return;
      }
      if (isStrEqual(argv[2], (uint8_t*) "REGISTER_NOTIFICATION_RSP"))
      {
         if (isStrEqual(argv[3], (uint8_t*) "CHANGED"))
         {

            if (isStrEqual(argv[4], (uint8_t*) "TRACK_CHANGED"))
            {
               bt_GetAVRCP_metadata();
               bt_TrackChangedEventSubscribe();
               return;
            }
            if (isStrEqual(argv[4], (uint8_t*) "PLAYBACK_STATUS_CHANGED"))
            {
               bt_PlaybackStatusEventSubscribe();

               if (isStrEqual(argv[5], (uint8_t*) "PLAYING"))
               {
                  playbackState = play;
               }
               if (isStrEqual(argv[5], (uint8_t*) "PAUSED"))
               {
                  playbackState = pause;
               }
               if (isStrEqual(argv[5], (uint8_t*) "STOPPED"))
               {
                  playbackState = stop;
               }
            }
         }
         /**/

      }
      return;
   }

   if (isStrEqual(argv[0], (uint8_t*) "CONNECT"))
   {
      if (isStrEqual(argv[2], (uint8_t*) "AVRCP"))
      {
         bt_TrackChangedEventSubscribe();
         bt_PlaybackStatusEventSubscribe();
         bt_GetAVRCP_metadata();
         bt_Play();
      }
      return;
   }

   if (isStrEqual(argv[0], (uint8_t*) "A2DP"))
   {
      if (isStrEqual(argv[1], (uint8_t*) "STREAMING"))
      {
         if (isStrEqual(argv[2], (uint8_t*) "STOP"))
         {
            //ClearDisplayString();
            //resetDisplayState();
         }
         if (isStrEqual(argv[2], (uint8_t*) "START"))
         {
            bt_TrackChangedEventSubscribe();
            bt_PlaybackStatusEventSubscribe();
            bt_GetAVRCP_metadata();
            //bt_Play();
         }
      }
      return;
   }
}
uint16_t token_lenght;
void Parse_init()
{
   memset(buf, 0x00, BUFF_SIZE);
   memset(argv, 0x00, BUFF_SIZE);
   parsing = 1;
   str_idx = 0;
   argc = 1;
   argv[0] = buf;
   text_flag = 0;

}

void Parse(uint8_t ch)
{

   if (str_idx >= BUFF_SIZE || argc >= ARGS_COUNT)
      return;
   if (ch == '\r')
      return;
   buf[str_idx] = ch;

   if (buf[str_idx] == '\n')
   {
      buf[str_idx] = 0;
      parsing = 0;
      HandleParseData();
      return;
   }

   if (text_flag)
   {
      if (token_lenght--)
      {
         str_idx++;
         return;
      }

      buf[str_idx] = 0;
      str_idx++;
      text_flag = 0;
   }

   if (buf[str_idx] == '\"')
   {

      argv[argc - 1] = &buf[str_idx + 1];
      str_idx++;
      token_lenght = _atoi(argv[argc - 2]);

      text_flag = 1;
      return;
   }

   if (buf[str_idx] == ' ' && !text_flag)
   {
      argv[argc] = &buf[str_idx + 1];
      argc++;
      buf[str_idx] = 0;
   }
   str_idx++;
}

uint8_t isStrEqual(uint8_t* s1, uint8_t* s2)
{
   uint8_t i = 0;
   while (s1[i] == s2[i] && s1[i] != '\0' && s2[i] != '\0')
   {
      i++;
   }
   if (s1[i] == '\0' && s2[i] == '\0')
      return 1;
   return 0;
}

uint16_t unicodeToChar(uint16_t w_char)
{

   uint16_t ch = 0;
   switch (w_char)
   {
   case 0xD090:
      ch = (uint16_t) 'A';
      break;
   case 0xD091:
      ch = (uint16_t) 'B';
      break;
   case 0xD092:
      ch = (uint16_t) 'V';
      break;
   case 0xD093:
      ch = (uint16_t) 'G';
      break;
   case 0xD094:
      ch = (uint16_t) 'D';
      break;
   case 0xD095:
      ch = (uint16_t) 'E';
      break;
   case 0xD096:
      ch = ((uint16_t) 'Z') << 8 | 'h';
      break;
   case 0xD097:
      ch = (uint16_t) 'Z';
      break;
   case 0xD098:
      ch = (uint16_t) 'I';
      break;
   case 0xD099:
      ch = (uint16_t) 'Y';
      break;
   case 0xD09A:
      ch = (uint16_t) 'K';
      break;
   case 0xD09B:
      ch = (uint16_t) 'L';
      break;
   case 0xD09C:
      ch = (uint16_t) 'M';
      break;
   case 0xD09D:
      ch = (uint16_t) 'N';
      break;
   case 0xD09E:
      ch = (uint16_t) 'O';
      break;
   case 0xD09F:
      ch = (uint16_t) 'P';
      break;
   case 0xD0A0:
      ch = (uint16_t) 'R';
      break;
   case 0xD0A1:
      ch = (uint16_t) 'S';
      break;
   case 0xD0A2:
      ch = (uint16_t) 'T';
      break;
   case 0xD0A3:
      ch = (uint16_t) 'U';
      break;
   case 0xD0A4:
      ch = (uint16_t) 'F';
      break;
   case 0xD0A5:
      ch = (uint16_t) 'H';
      break;
   case 0xD0A6:
      ch = ((uint16_t) 'T') << 8 | 's';
      break;
   case 0xD0A7:
      ch = ((uint16_t) 'C') << 8 | 'h';
      break;
   case 0xD0A8:
      ch = ((uint16_t) 'S') << 8 | 'h';
      break;
   case 0xD0A9:
      ch = ((uint16_t) 'S') << 8 | 'h';
      break;
   case 0xD0AA:
      ch = (uint16_t) '\'';
      break;
   case 0xD0AB:
      ch = (uint16_t) 'Y';
      break;
   case 0xD0AC:
      ch = (uint16_t) '\'';
      break;
   case 0xD0AD:
      ch = (uint16_t) 'E';
      break;
   case 0xD0AE:
      ch = ((uint16_t) 'Y') << 8 | 'u';
      break;
   case 0xD0AF:
      ch = ((uint16_t) 'Y') << 8 | 'a';
      break;
   case 0xD0B0:
      ch = (uint16_t) 'a';
      break;
   case 0xD0B1:
      ch = (uint16_t) 'b';
      break;
   case 0xD0B2:
      ch = (uint16_t) 'v';
      break;
   case 0xD0B3:
      ch = (uint16_t) 'g';
      break;
   case 0xD0B4:
      ch = (uint16_t) 'd';
      break;
   case 0xD0B5:
      ch = (uint16_t) 'e';
      break;
   case 0xD0B6:
      ch = ((uint16_t) 'z' << 8) | 'h';
      break;
   case 0xD0B7:
      ch = (uint16_t) 'z';
      break;
   case 0xD0B8:
      ch = (uint16_t) 'i';
      break;
   case 0xD0B9:
      ch = (uint16_t) 'y';
      break;
   case 0xD0BA:
      ch = (uint16_t) 'k';
      break;
   case 0xD0BB:
      ch = (uint16_t) 'l';
      break;
   case 0xD0BC:
      ch = (uint16_t) 'm';
      break;
   case 0xD0BD:
      ch = (uint16_t) 'n';
      break;
   case 0xD0BE:
      ch = (uint16_t) 'o';
      break;
   case 0xD0BF:
      ch = (uint16_t) 'p';
      break;
   case 0xD180:
      ch = (uint16_t) 'r';
      break;
   case 0xD181:
      ch = (uint16_t) 's';
      break;
   case 0xD182:
      ch = (uint16_t) 't';
      break;
   case 0xD183:
      ch = (uint16_t) 'u';
      break;
   case 0xD184:
      ch = (uint16_t) 'f';
      break;
   case 0xD185:
      ch = (uint16_t) 'h';
      break;
   case 0xD186:
      ch = ((uint16_t) 't') << 8 | 's';
      break;
   case 0xD187:
      ch = ((uint16_t) 'c') << 8 | 'h';
      break;
   case 0xD188:
      ch = ((uint16_t) 's') << 8 | 'h';
      break;
   case 0xD189:
      ch = ((uint16_t) 's') << 8 | 'h';
      break;
   case 0xD18A:
      ch = (uint16_t) '\'';
      break;
   case 0xD18B:
      ch = (uint16_t) 'y';
      break;
   case 0xD18C:
      ch = (uint16_t) '\'';
      break;
   case 0xD18D:
      ch = (uint16_t) 'e';
      break;
   case 0xD18E:
      ch = ((uint16_t) 'y') << 8 | 'u';
      break;
   case 0xD18F:
      ch = ((uint16_t) 'y') << 8 | 'a';
      break;
   case 0xD081:
      ch = ((uint16_t) 'Y') << 8 | 'o';
      break;
   case 0xD191:
      ch = ((uint16_t) 'y') << 8 | 'o';
      break;
   case 0xE280:
      ch = (uint16_t) '-';
      break;
   default:
      ch = (uint16_t) ' ';
   }
   return ch;
}
