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

uint8_t PN532RevK::inData(uint8_t *send, uint8_t sendLength, uint8_t *response, uint8_t *responseLength, int timeout)
{
  uint8_t buf[2];
  buf[0] = 0x40; // PN532_COMMAND_INDATAEXCHANGE;
  buf[1] = 1;

  if (HAL(writeCommand)(buf, 2, send, sendLength)) {
    return -1;
  }

  int16_t status = HAL(readResponse)(response, *responseLength, timeout);
  if (status < 0) {
    return -2;
  }

  if ((response[0] & 0x3f) != 0) {
    return response[0];
  }

  uint8_t length = status;
  length -= 1;

  *responseLength = length;
  
  if (length > *responseLength) {
    length = *responseLength; // silent truncation...
  }

  for (uint8_t i = 0; i < length; i++) {
    response[i] = response[i + 1];
  }


  return 0;
}
