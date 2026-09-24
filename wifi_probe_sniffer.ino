#include "WiFi.h"
#include "esp_wifi.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define BUTTON_CLEAR 19
#define BUTTON_NEXT 18
#define MAX_TRACKED 50
#define MED_COUNT 3
#define MED_GAP 1800000
#define HIGH_COUNT 5
#define HIGH_GAP 3600000
#define TOTAL_SCREENS 4
#define QUEUE_SIZE 20
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool fsAvailable=false;

struct ProbePacket {
  uint8_t mac[6];
  char ssid[33];
  int rssi;
  bool randomized;
};

struct MACEntry {
  String mac;
  unsigned long firstSeen;
  unsigned long lastSeen;
  int count;
  int rssi;
  bool randomized;
  String vendor;
};

QueueHandle_t probeQueue;

int probeCount=0;
String lastSSID="---";
int lastRSSI=0;
String lastMAC="---";
String lastThreat="NONE";
String lastVendor="---";
String lastProximity="---";
bool lastRandomized=false;
int lastSeen=0;
int currentScreen=0;
unsigned long bootTime=0;
int uniqueCount=0;

MACEntry seenMACs[MAX_TRACKED];
int seenTotal=0;

struct OUIEntry {
  const char prefix[9];
  const char vendor[20];
};

const OUIEntry ouiTable[] PROGMEM = {
  {"00:00:0c", "Cisco"},
  {"00:03:93", "Apple"},
  {"00:05:02", "Apple"},
  {"00:0a:27", "Apple"},
  {"00:0a:95", "Apple"},
  {"00:11:24", "Apple"},
  {"00:14:51", "Apple"},
  {"00:16:cb", "Apple"},
  {"00:17:f2", "Apple"},
  {"00:19:e3", "Apple"},
  {"00:1b:63", "Apple"},
  {"00:1c:b3", "Apple"},
  {"00:1d:4f", "Apple"},
  {"00:1e:52", "Apple"},
  {"00:1e:c2", "Apple"},
  {"00:1f:5b", "Apple"},
  {"00:1f:f3", "Apple"},
  {"00:21:e9", "Apple"},
  {"00:22:41", "Apple"},
  {"00:23:12", "Apple"},
  {"00:23:32", "Apple"},
  {"00:23:6c", "Apple"},
  {"00:23:df", "Apple"},
  {"00:24:36", "Apple"},
  {"00:25:00", "Apple"},
  {"00:25:4b", "Apple"},
  {"00:25:bc", "Apple"},
  {"00:26:08", "Apple"},
  {"00:26:b0", "Apple"},
  {"00:26:bb", "Apple"},
  {"00:50:f2", "Microsoft"},
  {"00:15:5d", "Microsoft"},
  {"00:17:fa", "Microsoft"},
  {"00:1d:d8", "Microsoft"},
  {"28:18:78", "Samsung"},
  {"30:07:4d", "Samsung"},
  {"38:aa:3c", "Samsung"},
  {"44:a7:cf", "Samsung"},
  {"4c:bc:a5", "Samsung"},
  {"50:01:bb", "Samsung"},
  {"54:88:0e", "Samsung"},
  {"78:40:e4", "Samsung"},
  {"8c:77:12", "Samsung"},
  {"a0:07:98", "Samsung"},
  {"f4:7b:5e", "Samsung"},
  {"3c:5a:b4", "Google"},
  {"54:60:09", "Google"},
  {"f4:f5:e8", "Google"},
  {"00:1a:11", "Google"},
  {"94:eb:2c", "Espressif"},
  {"a4:cf:12", "Espressif"},
  {"b4:e6:2d", "Espressif"},
  {"cc:50:e3", "Espressif"},
  {"24:6f:28", "Espressif"},
  {"30:ae:a4", "Espressif"},
  {"00:13:ef", "Huawei"},
  {"00:18:82", "Huawei"},
  {"00:1e:10", "Huawei"},
  {"00:25:9e", "Huawei"},
  {"00:34:fe", "Huawei"},
  {"04:bd:70", "Huawei"},
  {"28:6e:d4", "Xiaomi"},
  {"34:80:b3", "Xiaomi"},
  {"38:a4:ed", "Xiaomi"},
  {"50:8f:4c", "Xiaomi"},
  {"64:09:80", "Xiaomi"},
  {"74:23:44", "Xiaomi"},
  {"8c:be:be", "Xiaomi"},
  {"ac:c1:ee", "Xiaomi"},
  {"00:17:c8", "OnePlus"},
  {"04:4e:af", "OnePlus"},
  {"00:17:88", "Philips Hue"},
  {"18:b4:30", "Nest"},
  {"64:16:66", "Amazon"},
  {"68:37:e9", "Amazon"},
  {"74:c2:46", "Amazon"},
  {"a0:02:dc", "Amazon"},
  {"00:1c:bf", "Intel"},
  {"00:21:6a", "Intel"},
  {"00:24:d7", "Intel"},
  {"04:0e:3c", "Intel"},
  {"00:00:ab", "Motorola"},
  {"00:08:a3", "Motorola"},
  {"00:0e:6d", "Motorola"},
};
int ouiCount=sizeof(ouiTable)/sizeof(ouiTable[0]);

String lookupVendor(uint8_t* mac){
  char prefix[9];
  sprintf(prefix, "%02x:%02x:%02x", mac[0], mac[1], mac[2]);
  for (int i=0; i<ouiCount; i++){
    if (strcmp(prefix, ouiTable[i].prefix)==0){
      return String(ouiTable[i].vendor);
    }
  }
  return "Unknown";
}

bool isRandomizedMAC(uint8_t* mac){
  return (mac[0] & 0x02);
}

String getProximity(int rssi){
  if (rssi>=-60) return "NEAR";
  if (rssi>=-80) return "MID";
  return "FAR";
}

int trackMAC(String mac, int rssi, bool randomized, String vendor){
  unsigned long now=millis();
  for (int i=0; i<seenTotal; i++){
    if (seenMACs[i].mac==mac){
      seenMACs[i].lastSeen=now;
      seenMACs[i].count++;
      seenMACs[i].rssi=rssi;
      return seenMACs[i].count;
    }
  }
  if (seenTotal<MAX_TRACKED){
    seenMACs[seenTotal].mac=mac;
    seenMACs[seenTotal].firstSeen=now;
    seenMACs[seenTotal].lastSeen=now;
    seenMACs[seenTotal].count=1;
    seenMACs[seenTotal].rssi=rssi;
    seenMACs[seenTotal].randomized=randomized;
    seenMACs[seenTotal].vendor=vendor;
    seenTotal++;
    uniqueCount++;
  }
  return 1;
}

String getThreatLevel(String mac){
  for (int i=0; i<seenTotal; i++){
    if (seenMACs[i].mac==mac){
      unsigned long gap=seenMACs[i].lastSeen-seenMACs[i].firstSeen;
      int count=seenMACs[i].count;
      if (count>=HIGH_COUNT && gap>=HIGH_GAP) return "HIGH";
      if (count>=MED_COUNT && gap>=MED_GAP) return "MED";
      if (count>=2) return "LOW";
    }
  }
  return "NONE";
}

MACEntry getMostSuspicious(){
  MACEntry best;
  best.mac="---";
  best.count=0;
  best.rssi=0;
  best.firstSeen=0;
  best.lastSeen=0;
  best.randomized=false;
  best.vendor="---";
  int bestScore=0;
  for (int i=0; i<seenTotal; i++){
    int score=seenMACs[i].count;
    unsigned long gap=seenMACs[i].lastSeen-seenMACs[i].firstSeen;
    if (gap>=HIGH_GAP) score+=10;
    else if (gap>=MED_GAP) score+=5;
    if (score>bestScore){
      bestScore=score;
      best=seenMACs[i];
    }
  }
  return best;
}

void clearTracked(){
  for (int i=0; i<seenTotal; i++){
    seenMACs[i].mac="";
    seenMACs[i].count=0;
    seenMACs[i].firstSeen=0;
    seenMACs[i].lastSeen=0;
    seenMACs[i].rssi=0;
    seenMACs[i].randomized=false;
    seenMACs[i].vendor="";
  }
  seenTotal=0;
  uniqueCount=0;
}

String getUptime(){
  unsigned long elapsed=(millis()-bootTime)/1000;
  int hrs=elapsed/3600;
  int mins=(elapsed%3600)/60;
  int secs=elapsed%60;
  char buf[12];
  sprintf(buf, "%02d:%02d:%02d", hrs, mins, secs);
  return String(buf);
}

void logToFS(String mac, String ssid, int rssi, String vendor, bool randomized){
  if (!fsAvailable) return;
  File f=LittleFS.open("/probes.csv", "a");
  if (!f) return;
  f.printf("%lu,%s,%s,%d,%s,%s\n",
           millis(), mac.c_str(),
           ssid.length()>0 ? ssid.c_str() : "(hidden)",
           rssi, vendor.c_str(),
           randomized ? "rand" : "real");
  f.close();
}

void printSerial(){
  Serial.println("-----------------------------------");
  Serial.printf("PROBE #%d\n", probeCount);
  Serial.printf("MAC:      %s%s\n", lastMAC.c_str(), lastRandomized ? " (randomized)" : "");
  Serial.printf("Vendor:   %s\n", lastVendor.c_str());
  Serial.printf("SSID:     %s\n", lastSSID.length()>0 ? lastSSID.c_str() : "(hidden)");
  Serial.printf("RSSI:     %d dBm\n", lastRSSI);
  Serial.printf("Proximity:%s\n", lastProximity.c_str());
  Serial.printf("Seen:     %dx\n", lastSeen);
  Serial.printf("Threat:   %s\n", lastThreat.c_str());
  Serial.printf("Unique:   %d devices\n", uniqueCount);
  Serial.printf("Uptime:   %s\n", getUptime().c_str());
  Serial.printf("Logged:   %s\n", fsAvailable ? "YES" : "NO (no storage)");
  if (lastThreat=="HIGH")     Serial.println("!! YOU ARE BEING FOLLOWED !!");
  else if (lastThreat=="MED") Serial.println("!! DEVICE KEEPS REAPPEARING !!");
  else if (lastThreat=="LOW") Serial.println("!! SEEN NEARBY TWICE !!");
  Serial.println("-----------------------------------");
}

void drawScreen0(){
  display.setCursor(0,0);
  if (lastThreat=="HIGH")     display.println("!! BEING FOLLOWED !!");
  else if (lastThreat=="MED") display.println("!! REAPPEARING !!");
  else if (lastThreat=="LOW") display.println("!! SEEN TWICE !!");
  else                        display.println("Wifi Probe Sniffer");
  display.setCursor(0,12);
  display.print("SSID: ");
  display.println(lastSSID.length()>0 ? lastSSID : "(hidden)");
  display.setCursor(0,24);
  display.print("MAC:  ");
  display.println(lastMAC);
  display.setCursor(0,36);
  display.print("Vendor: ");
  display.println(lastVendor);
  display.setCursor(0,48);
  display.print("RSSI:");
  display.print(lastRSSI);
  display.print(" ");
  display.print(lastProximity);
  display.print(" T:");
  display.println(lastThreat);
}

void drawScreen1(){
  display.setCursor(0,0);
  display.println("--- Stats ---");
  display.setCursor(0,12);
  display.print("Total Probes: ");
  display.println(probeCount);
  display.setCursor(0,24);
  display.print("Unique Devices: ");
  display.println(uniqueCount);
  display.setCursor(0,36);
  display.print("Uptime: ");
  display.println(getUptime());
  display.setCursor(0,48);
  display.print("Logged: ");
  display.println(fsAvailable ? "YES" : "NO");
}

void drawScreen2(){
  MACEntry s=getMostSuspicious();
  display.setCursor(0,0);
  display.println("--- Most Suspicious ---");
  display.setCursor(0,12);
  display.print("MAC: ");
  display.println(s.mac);
  display.setCursor(0,24);
  display.print("Vendor: ");
  display.println(s.vendor);
  display.setCursor(0,36);
  display.print("Seen: ");
  display.print(s.count);
  display.print("x ");
  display.println(getThreatLevel(s.mac));
  display.setCursor(0,48);
  display.print("Rand: ");
  display.print(s.randomized ? "YES" : "NO");
  display.print("  ");
  display.println(getProximity(s.rssi));
}

void drawScreen3(){
  display.setCursor(0,0);
  display.println("--- Last Probe ---");
  display.setCursor(0,12);
  display.print("MAC: ");
  display.println(lastMAC);
  display.setCursor(0,24);
  display.print("Proximity: ");
  display.println(lastProximity);
  display.setCursor(0,36);
  display.print("Vendor: ");
  display.println(lastVendor);
  display.setCursor(0,48);
  display.print("Rand MAC: ");
  display.println(lastRandomized ? "YES" : "NO");
}

void updateDisp(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  if (currentScreen==0)      drawScreen0();
  else if (currentScreen==1) drawScreen1();
  else if (currentScreen==2) drawScreen2();
  else if (currentScreen==3) drawScreen3();
  display.display();
}

void probeWorkerTask(void* pvParameters){
  ProbePacket pkt;
  while (true){
    if (xQueueReceive(probeQueue, &pkt, portMAX_DELAY)==pdTRUE){
      String mac="";
      for (int i=0; i<6; i++){
        if (pkt.mac[i]<0x10) mac+="0";
        mac+=String(pkt.mac[i], HEX);
        if (i<5) mac+=":";
      }

      String ssid=strlen(pkt.ssid)>0 ? String(pkt.ssid) : "";
      String vendor=lookupVendor(pkt.mac);

      lastMAC=mac;
      lastSSID=ssid;
      lastRSSI=pkt.rssi;
      lastRandomized=pkt.randomized;
      lastVendor=vendor;
      lastProximity=getProximity(pkt.rssi);
      probeCount++;
      lastSeen=trackMAC(mac, pkt.rssi, pkt.randomized, vendor);
      lastThreat=getThreatLevel(mac);

      logToFS(mac, ssid, pkt.rssi, vendor, pkt.randomized);
      printSerial();
      updateDisp();
    }
  }
}

void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type){
  if (type!=WIFI_PKT_MGMT) return;

  wifi_promiscuous_pkt_t* pkt=(wifi_promiscuous_pkt_t*)buf;
  uint8_t* data=pkt->payload;

  if (data[0]!=0x40) return;

  ProbePacket p;
  memcpy(p.mac, data+10, 6);

  uint8_t ssid_len=data[25];
  memset(p.ssid, 0, 33);
  if (ssid_len>0 && ssid_len<=32){
    memcpy(p.ssid, data+26, ssid_len);
  }

  p.rssi=pkt->rx_ctrl.rssi;
  p.randomized=isRandomizedMAC(p.mac);

  xQueueSendFromISR(probeQueue, &p, NULL);
}

void hopChannel(){
  static uint8_t channel=1;
  channel=(channel%13)+1;
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}

void setup(){
  Serial.begin(115200);
  delay(1000);
  bootTime=millis();

  Serial.println("-----------------------------------");
  Serial.println("Wifi Probe Sniffer");
  Serial.println("Initializing.....");
  Serial.println("-----------------------------------");

  if (LittleFS.begin(true)){
    fsAvailable=true;
    Serial.println("LittleFS mounted, logging enabled");
    if (!LittleFS.exists("/probes.csv")){
      File f=LittleFS.open("/probes.csv", "w");
      if (f){
        f.println("uptime_ms,mac,ssid,rssi,vendor,mac_type");
        f.close();
      }
    }
  } else {
    Serial.println("LittleFS failed, logging disabled");
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("OLED display output failed, continuing without it");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,20);
    display.println("Wifi Probe Sniffer");
    display.setCursor(0,35);
    display.println("Initializing...");
    display.display();
    delay(2000);
  }

  probeQueue=xQueueCreate(QUEUE_SIZE, sizeof(ProbePacket));

  xTaskCreatePinnedToCore(
    probeWorkerTask,
    "probeWorker",
    8192,
    NULL,
    1,
    NULL,
    0
  );

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&sniffer_callback);

  Serial.println("Probe Sniffer ready!");
  Serial.println("Listening for probe requests.....");
  Serial.println("-----------------------------------");
  updateDisp();

  pinMode(BUTTON_CLEAR, INPUT_PULLUP);
  pinMode(BUTTON_NEXT, INPUT_PULLUP);
}

void loop(){
  static unsigned long lastHop=0;
  if (millis()-lastHop>=500){
    hopChannel();
    lastHop=millis();
  }

  if (digitalRead(BUTTON_NEXT)==LOW){
    currentScreen=(currentScreen+1)%TOTAL_SCREENS;
    updateDisp();
    delay(300);
  }

  if (digitalRead(BUTTON_CLEAR)==LOW){
    probeCount=0;
    lastSSID="---";
    lastMAC="---";
    lastRSSI=0;
    lastSeen=0;
    lastThreat="NONE";
    lastVendor="---";
    lastProximity="---";
    lastRandomized=false;
    clearTracked();
    if (fsAvailable){
      LittleFS.remove("/probes.csv");
      File f=LittleFS.open("/probes.csv", "w");
      if (f){
        f.println("uptime_ms,mac,ssid,rssi,vendor,mac_type");
        f.close();
      }
    }
    Serial.println("-----------------------------------");
    Serial.println("Log cleared!");
    Serial.println("-----------------------------------");
    updateDisp();
    delay(500);
  }
}