#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

const int buttonPin = 2;       
int blockWidth = 8;            
int currentY = 7;              
int blockX = 0;                
int direction = 1;             
unsigned long lastMoveTime = 0;
int gameSpeed = 220;           

// Pengunci double click berbasis waktu (Debounce)
unsigned long lastButtonTime = 0; 
const unsigned long debounceDelay = 250; // Jeda aman antar klik (250 milidetik)

// PERBAIKAN: Ditambahkan [8] agar dikenali sebagai array 8 slot
uint8_t savedLeft[8] = {0};
uint8_t savedRight[8] = {0};

bool gameOver = false;
bool gameWon = false;

// PERBAIKAN: Ditambahkan [8][12] agar menjadi array 2D matrix layar yang benar
uint8_t frame[8][12] = {0};

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  matrix.begin();
}

void loop() {
  // 1. Logika Cek Status Akhir Game
  if (gameOver || gameWon) {
    renderStatusScreen(); 
    if (digitalRead(buttonPin) == LOW) {
      if (millis() - lastButtonTime > debounceDelay) {
        lastButtonTime = millis();
        blockWidth = 8; currentY = 7; blockX = 0; direction = 1; gameSpeed = 220;
        gameOver = false; gameWon = false;
      }
    }
    return;
  }

  // 2. Logika Pergerakan Otomatis (Frame Rate Controller)
  if (millis() - lastMoveTime > (unsigned long)gameSpeed) {
    lastMoveTime = millis();
    blockX += direction;
    if (blockX <= 0 || blockX + blockWidth >= 12) direction *= -1;
    renderGame();
  }

  // 3. Logika Eksekusi Tombol dengan Sistem Waktu Pengunci (Anti Double Click)
  if (digitalRead(buttonPin) == LOW) {
    if (millis() - lastButtonTime > debounceDelay) {
      lastButtonTime = millis(); // Kunci waktu penekanan saat ini

      int currentLeft = blockX;
      int currentRight = blockX + blockWidth - 1;

      if (currentY < 7) {
        if (currentRight < savedLeft[currentY + 1] || currentLeft > savedRight[currentY + 1]) {
          gameOver = true;
          return;
        }
      }

      savedLeft[currentY] = currentLeft;
      savedRight[currentY] = currentRight;

      currentY--; 
      blockWidth--; 

      // Rumus kelajuan yang stabil dan enteng
      gameSpeed = 220 - ((7 - currentY) * 25);
      if (gameSpeed < 45) gameSpeed = 45; 

      blockX = 0;
      direction = 1;

      if (currentY < 0) gameWon = true;
    }
  }
}

// --- FUNGSI GRAFIS EFISIEN ---

void renderGame() {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 12; x++) {
      if (y > currentY) {
        frame[y][x] = (x >= savedLeft[y] && x <= savedRight[y]) ? 1 : 0;
      } else if (y == currentY) {
        frame[y][x] = (x >= blockX && x < blockX + blockWidth) ? 1 : 0;
      } else {
        frame[y][x] = 0;
      }
    }
  }
  matrix.renderBitmap(frame, 8, 12);
}

void renderStatusScreen() {
  static unsigned long lastFlash = 0;
  static bool flashState = false;
  
  if (millis() - lastFlash > 250) {
    lastFlash = millis();
    flashState = !flashState;
    
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 12; x++) {
        if (gameOver) {
          frame[y][x] = (flashState && (x == y + 2 || x == 9 - y)) ? 1 : 0;
        } else if (gameWon) {
          frame[y][x] = flashState ? 1 : 0;
        }
      }
    }
    matrix.renderBitmap(frame, 8, 12);
  }
}
