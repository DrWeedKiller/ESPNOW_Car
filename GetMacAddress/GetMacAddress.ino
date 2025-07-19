#include "WiFi.h"
#include <esp_system.h>  // ESP.getEfuseMac()

static char   _mac[13]      = {0};   // 12 chars + '\0'

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);
  Serial.println(WiFi.macAddress());
  uint64_t raw = ESP.getEfuseMac();
  for (int i = 0; i < 6; ++i) {
    uint8_t b = (raw >> (8 * (5 - i))) & 0xFF;
    sprintf(&_mac[i * 2], "%02X", b);
  }
  _mac[12] = '\0';
  Serial.print(_mac);
}

void loop()
{

}
