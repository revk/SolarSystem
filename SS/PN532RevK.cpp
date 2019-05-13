#include "PN532.h"
#include "PN532RevK.h"

#define HAL(func)   (_interface->func)

uint8_t PN532RevK::getGeneralStatus(int timeout)
{
  uint8_t buf[10];
  buf[0] = PN532_COMMAND_GETGENERALSTATUS;
  if (HAL(writeCommand)(buf, 1))
    return 0;

  if (HAL(readResponse)(buf, sizeof(buf), timeout) < 0)
    return 0;

  return buf[0];
}

uint8_t PN532RevK::diagnose6(int timeout)
{ // Test 6 is card presence detection
  uint8_t buf[10];
  buf[0] = PN532_COMMAND_DIAGNOSE;
  buf[1] = 6;

  if (HAL(writeCommand)(buf, 2))
    return 0xFF;

  if (HAL(readResponse)(buf, sizeof(buf), timeout) < 0)
    return 0xFF;

  return buf[0];
}
