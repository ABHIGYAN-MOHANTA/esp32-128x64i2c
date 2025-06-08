#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Wi-Fi credentials
const char* ssid     = "SOA UNIVERSITY 2.4";
const char* password = "iter@bh1";

// NTP time settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // IST offset (5h 30m = 19800 seconds)
const int daylightOffset_sec = 0;

void setup() {
  Serial.begin(115200);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1); // Smallest font
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    retry++;
    Serial.print(".");
  }

  display.clearDisplay();
  if (WiFi.status() == WL_CONNECTED) {
    display.setCursor(0, 0);
    display.println("WiFi connected");
    display.println(WiFi.localIP());
    display.display();

    // Get time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(1000);
  } else {
    display.setCursor(0, 0);
    display.println("WiFi failed.");
    display.display();
  }

  delay(2000); // Pause to show connection status
  display.clearDisplay();
}

void showTime() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(timeStr);
    display.setCursor( 128/2, 0);
    display.println("_abhigyan");
    display.display();
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Syncing Time");
    display.display();
  }
}

void loop() {
  showTime();
  delay(1000);
}
