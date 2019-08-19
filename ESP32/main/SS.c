/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "../components/ESP32RevK/revk.h"

#include "door.h"
#include "input.h"
#include "output.h"
#include "nfc.h"
#include "ranger.h"
#include "keypad.h"

const char *app_setting(const char *tag,unsigned int len,const unsigned char *value)
{
return ""; // Unknown
}

const char *app_command(const char *tag,unsigned int len,const unsigned char *value)
{
return ""; // Unknown
}

void app_main()
{
printf("Main SS app\n");
revk_init(__FILE__,__DATE__,__TIME__,&app_setting,&app_command);
}
