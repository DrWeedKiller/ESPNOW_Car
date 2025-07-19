#include <esp_now.h>
#include <WiFi.h>

#define X_AXIS_PIN 32
#define Y_AXIS_PIN 33
#define SWITCH_PIN 25

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t receiverMacAddress[] = {0x30,0xc6,0xF7,0x43,0x45,0x1C};  //1C:45:43:F7:C6:30

struct PacketData
{
  byte xAxisValue;
  byte yAxisValue;
  byte switchPressed;
};
PacketData data;

//This function is used to map 0-4095 joystick value to 0-254. hence 127 is the center value which we send.
//It also adjust the deadband in joystick.
//Jotstick values range from 0-4095. But its center value is not always 2047. It is little different.
//So we need to add some deadband to center value. in our case 1800-2200. Any value in this deadband range is mapped to center 127.
int mapAndAdjustJoystickDeadBandValues(int value, bool reverse)
{
  if (value >= 2200)
  {
    value = map(value, 2200, 4095, 127, 254);
  }
  else if (value <= 1800)
  {
    value = map(value, 1800, 0, 127, 0);  
  }
  else
  {
    value = 127;
  }

  if (reverse)
  {
    value = 254 - value;
  }
  return value;
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("Last Packet Send Status: ");
  Serial.print(status);
  Serial.print(" (");
  
  switch(status) {
    case ESP_NOW_SEND_SUCCESS:
      Serial.println("SUCCESS)");
      break;
    case ESP_NOW_SEND_FAIL:
      Serial.println("FAIL - Check MAC address and receiver)");
      break;
    default:
      Serial.println("UNKNOWN)");
      break;
  }
}

void printMacAddress() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.print("Transmitter MAC Address: ");
  for (int i = 0; i < 6; i++) {
    if (i > 0) Serial.print(":");
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
  }
  Serial.println();
}

void setup() 
{
  Serial.begin(115200);
  
  // Print this device's MAC address for debugging
  WiFi.mode(WIFI_STA);
  printMacAddress();
  
  Serial.print("Target receiver MAC: ");
  for (int i = 0; i < 6; i++) {
    if (i > 0) Serial.print(":");
    if (receiverMacAddress[i] < 16) Serial.print("0");
    Serial.print(receiverMacAddress[i], HEX);
  }
  Serial.println();

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else
  {
    Serial.println("Success: Initialized ESP-NOW");
  }

  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  else
  {
    Serial.println("Success: Added peer");
  } 

  pinMode(SWITCH_PIN, INPUT_PULLUP);
  
  Serial.println("Setup complete. Starting transmission...");
  delay(1000);
}
 
void loop() 
{
  data.xAxisValue = mapAndAdjustJoystickDeadBandValues(analogRead(X_AXIS_PIN), false);
  data.yAxisValue = mapAndAdjustJoystickDeadBandValues(analogRead(Y_AXIS_PIN), false);  
  data.switchPressed = false; 

  if (digitalRead(SWITCH_PIN) == LOW)
  {
    data.switchPressed = true;
  }
  
  // Print data being sent for debugging
  Serial.print("Sending - X: ");
  Serial.print(data.xAxisValue);
  Serial.print(", Y: ");
  Serial.print(data.yAxisValue);
  Serial.print(", Switch: ");
  Serial.print(data.switchPressed);
  Serial.print(" -> ");
  
  esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *) &data, sizeof(data));
  if (result == ESP_OK) 
  {
    Serial.print("Send initiated successfully");
  }
  else 
  {
    Serial.print("Error initiating send: ");
    Serial.print(result);
  }    
  Serial.println();
  
  if (data.switchPressed == true)
  {
    delay(500);
  }
  else
  {
    delay(100); // Increased delay to reduce spam
  }
}