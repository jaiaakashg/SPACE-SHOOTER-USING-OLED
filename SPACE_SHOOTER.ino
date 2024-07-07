#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define VRX_PIN 34
#define VRY_PIN 35
#define SW_PIN  23

int playerX = SCREEN_WIDTH / 2;
int playerY = SCREEN_HEIGHT - 10;


#define DEADZONE 200
#define MAX_PROJECTILES 10
int projectileX[MAX_PROJECTILES];
int projectileY[MAX_PROJECTILES];
bool projectileActive[MAX_PROJECTILES];


#define MAX_ENEMIES 5
int enemyX[MAX_ENEMIES];
int enemyY[MAX_ENEMIES];
int enemySpeed[MAX_ENEMIES];
int enemyType[MAX_ENEMIES];


bool rapidFireActive = false;
int rapidFireCooldown = 0;
int score = 0;

enum GameState {
  PLAYING,
  GAME_OVER
};

GameState gameState = PLAYING;

void setup() {
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  pinMode(VRX_PIN, INPUT);
  pinMode(VRY_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);


  display.clearDisplay();

  for (int i = 0; i < MAX_PROJECTILES; i++) {
    projectileActive[i] = false;
  }

  for (int i = 0; i < MAX_ENEMIES; i++) {
    respawnEnemy(i);
  }
}

void loop() {
  switch (gameState) {
    case PLAYING:
      updateGame();
      break;
    case GAME_OVER:
      gameOver();
      break;
  }
}

void updateGame() {
  int vrxValue = analogRead(VRX_PIN);
  int vryValue = analogRead(VRY_PIN);
  int swValue = digitalRead(SW_PIN);

  
  if (abs(vrxValue - 2048) > DEADZONE) {
    playerX += map(vrxValue, 0, 4095, -5, 5);
  }
  if (abs(vryValue - 2048) > DEADZONE) {
    playerY -= map(vryValue, 0, 4095, -5, 5);
  }

  playerX = constrain(playerX, 0, SCREEN_WIDTH - 1);
  playerY = constrain(playerY, 0, SCREEN_HEIGHT - 1);

  
  if (swValue == LOW) {
    shootProjectile();
  }


  updateProjectiles();

 
  updateEnemies();


  checkCollisions();

  
  handlePowerUps();


  displayGame();


  if (isGameOver()) {
    gameState = GAME_OVER;
  }


  delay(50);
}

void shootProjectile() {
  if (rapidFireActive || rapidFireCooldown <= 0) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
      if (!projectileActive[i]) {
        projectileX[i] = playerX;
        projectileY[i] = playerY - 5;
        projectileActive[i] = true;
        if (rapidFireActive) {
          rapidFireCooldown = 5; 
        }
        break;
      }
    }
  }
}

void updateProjectiles() {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (projectileActive[i]) {
      projectileY[i] -= 5;
      if (projectileY[i] < 0) {
        projectileActive[i] = false;
      }
    }
  }
}

void updateEnemies() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    enemyY[i] += enemySpeed[i];
    if (enemyY[i] > SCREEN_HEIGHT) {
      respawnEnemy(i);
    }
  }
}

void respawnEnemy(int index) {
  enemyX[index] = random(0, SCREEN_WIDTH);
  enemyY[index] = random(-SCREEN_HEIGHT, 0);
  enemySpeed[index] = random(1, 3);
  enemyType[index] = random(0, 3); 
}

void checkCollisions() {
  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (projectileActive[i]) {
      for (int j = 0; j < MAX_ENEMIES; j++) {
        if (abs(projectileX[i] - enemyX[j]) < 5 && abs(projectileY[i] - enemyY[j]) < 5) {
          projectileActive[i] = false;
          respawnEnemy(j);
          score += 10;
          
        }
      }
    }
  }
}

void handlePowerUps() {
  if (rapidFireCooldown > 0) {
    rapidFireCooldown--;
  }
}

void displayGame() {
  display.clearDisplay();

  display.drawLine(playerX, playerY - 8, playerX - 4, playerY + 4, SSD1306_WHITE);
  display.drawLine(playerX, playerY - 8, playerX + 4, playerY + 4, SSD1306_WHITE);
  display.drawLine(playerX - 4, playerY + 4, playerX + 4, playerY + 4, SSD1306_WHITE);

  for (int i = 0; i < MAX_PROJECTILES; i++) {
    if (projectileActive[i]) {
      display.drawFastVLine(projectileX[i], projectileY[i], 5, SSD1306_WHITE);
    }
  }

  for (int i = 0; i < MAX_ENEMIES; i++) {
    switch (enemyType[i]) {
      case 0:
        display.drawTriangle(enemyX[i], enemyY[i] - 5, enemyX[i] - 5, enemyY[i] + 5, enemyX[i] + 5, enemyY[i] + 5, SSD1306_WHITE);
        break;
      case 1:
        display.drawRect(enemyX[i] - 5, enemyY[i] - 5, 10, 10, SSD1306_WHITE);
        break;
      case 2:
        display.drawCircle(enemyX[i], enemyY[i], 5, SSD1306_WHITE);
        break;
    }
  }

  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.print("Score: ");
  display.print(score);

  
  display.display();
}

bool isGameOver() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (abs(playerX - enemyX[i]) < 5 && abs(playerY - enemyY[i]) < 5) {
      return true;
    }
  }
  return false;
}

void gameOver() {
  display.clearDisplay();
  display.setCursor(20, 20);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.println("Game Over!");
  display.display();

  score = 0;
  gameState = PLAYING;
  delay(5000); 

  for (int i = 0; i < MAX_ENEMIES; i++) {
    respawnEnemy(i);
  }
}

