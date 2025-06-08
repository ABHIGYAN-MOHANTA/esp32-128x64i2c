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
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

// Game of Life settings
#define GRID_WIDTH  16
#define GRID_HEIGHT 8
bool grid[GRID_HEIGHT][GRID_WIDTH];
bool newGrid[GRID_HEIGHT][GRID_WIDTH];

int cellWidth = SCREEN_WIDTH / GRID_WIDTH;   // 8
int cellHeight = (SCREEN_HEIGHT / 2) / GRID_HEIGHT;  // 4

void randomizeGrid() {
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      grid[y][x] = random(2); // 0 or 1
    }
  }
}

int countNeighbors(int x, int y) {
  int count = 0;
  for (int j = -1; j <= 1; j++) {
    for (int i = -1; i <= 1; i++) {
      if (i == 0 && j == 0) continue;
      int nx = x + i;
      int ny = y + j;
      if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
        count += grid[ny][nx];
      }
    }
  }
  return count;
}

void updateGrid() {
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      int neighbors = countNeighbors(x, y);
      if (grid[y][x]) {
        newGrid[y][x] = (neighbors == 2 || neighbors == 3);
      } else {
        newGrid[y][x] = (neighbors == 3);
      }
    }
  }
  memcpy(grid, newGrid, sizeof(grid));
}

void drawGridOutline() {
  int yOffset = SCREEN_HEIGHT / 2;
  for (int y = 0; y <= GRID_HEIGHT; y++) {
    display.drawLine(0, yOffset + y * cellHeight, SCREEN_WIDTH, yOffset + y * cellHeight, SSD1306_WHITE);
  }
  for (int x = 0; x <= GRID_WIDTH; x++) {
    display.drawLine(x * cellWidth, yOffset, x * cellWidth, SCREEN_HEIGHT, SSD1306_WHITE);
  }
}

void drawGridCells() {
  int yOffset = SCREEN_HEIGHT / 2;
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[y][x]) {
        display.fillRect(x * cellWidth + 1, yOffset + y * cellHeight + 1, cellWidth - 2, cellHeight - 2, SSD1306_WHITE);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

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
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(1000);
  } else {
    display.setCursor(0, 0);
    display.println("WiFi failed.");
    display.display();
  }

  delay(2000);
  display.clearDisplay();
  randomizeGrid();
}

void showTimeAndGame() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[10];
    char ampmStr[3];
    
    strftime(timeStr, sizeof(timeStr), "%I:%M:%S", &timeinfo);
    strftime(ampmStr, sizeof(ampmStr), "%p", &timeinfo);

    display.clearDisplay();

    // === Time ===
    display.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
    int16_t x = (SCREEN_WIDTH - w) / 2;
    int16_t y = 4;
    display.setCursor(x, y);
    display.println(timeStr);

    // === AM/PM ===
    display.setTextSize(1);
    display.getTextBounds(ampmStr, 0, 0, &x1, &y1, &w, &h);
    int16_t ampm_x = (SCREEN_WIDTH - w) / 2;
    int16_t ampm_y = y + 18;
    display.setCursor(ampm_x, ampm_y);
    display.println(ampmStr);

    // === Game of Life ===
    updateGrid();
    drawGridOutline();
    drawGridCells();

    display.display();
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Syncing Time");
    display.display();
  }
}

unsigned long lastResetTime = 0;
const unsigned long resetInterval = 5000;

void loop() {
  unsigned long currentTime = millis();
  
  // Reset grid every 5 seconds
  if (currentTime - lastResetTime >= resetInterval) {
    randomizeGrid();
    lastResetTime = currentTime;
  }

  showTimeAndGame();
  delay(400);
}
