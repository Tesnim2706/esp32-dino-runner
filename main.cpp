
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ==========================
// OLED CONFIGURATION
// ==========================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);

// ==========================
// PIN CONFIGURATION
// ==========================

#define BUTTON_PIN 18

// ==========================
// GAME SETTINGS
// ==========================

const int GROUND_Y = 55;
const int DINO_X = 15;

float dinoY = 45;
float velocity = 0;

const float GRAVITY = 0.65;
const float JUMP_FORCE = -7.5;

int cactusX = SCREEN_WIDTH;
int cactusHeight = 15;
int gameSpeed = 2;

int score = 0;

bool jumping = false;
bool gameOver = false;
bool started = false;

unsigned long lastFrame = 0;
const int FRAME_DELAY = 30;

// ==========================
// BUTTON DEBOUNCING
// ==========================

bool buttonPressed() {
    static int lastReading = HIGH;
    static int buttonState = HIGH;
    static unsigned long lastDebounceTime = 0;

    int reading = digitalRead(BUTTON_PIN);

    if (reading != lastReading) {
        lastDebounceTime = millis();
    }

    bool pressed = false;

    if (millis() - lastDebounceTime > 35) {
        if (reading != buttonState) {
            buttonState = reading;

            if (buttonState == LOW) {
                pressed = true;
            }
        }
    }

    lastReading = reading;

    return pressed;
}

// ==========================
// DRAW DINOSAUR
// ==========================

void drawDino(int x, int y) {

    // Body
    display.fillRect(x, y, 10, 10, SSD1306_WHITE);

    // Head
    display.fillRect(x + 5, y - 5, 8, 8, SSD1306_WHITE);

    // Eye
    display.drawPixel(x + 11, y - 2, SSD1306_BLACK);

    // Legs
    display.drawLine(
        x + 2, y + 10,
        x + 2, y + 13,
        SSD1306_WHITE
    );

    display.drawLine(
        x + 8, y + 10,
        x + 8, y + 13,
        SSD1306_WHITE
    );
}

// ==========================
// DRAW CACTUS
// ==========================

void drawCactus(int x) {

    // Main trunk
    display.fillRect(
        x + 2,
        GROUND_Y - cactusHeight,
        4,
        cactusHeight,
        SSD1306_WHITE
    );

    // Left arm
    display.fillRect(
        x,
        GROUND_Y - 10,
        3,
        5,
        SSD1306_WHITE
    );

    // Right arm
    display.fillRect(
        x + 5,
        GROUND_Y - 7,
        3,
        5,
        SSD1306_WHITE
    );
}

// ==========================
// COLLISION DETECTION
// ==========================

bool collision() {

    int dinoLeft = DINO_X;
    int dinoRight = DINO_X + 13;

    int dinoTop = (int)dinoY - 5;
    int dinoBottom = (int)dinoY + 13;

    int cactusLeft = cactusX;
    int cactusRight = cactusX + 8;
    int cactusTop = GROUND_Y - cactusHeight;

    return (
        dinoRight > cactusLeft &&
        dinoLeft < cactusRight &&
        dinoBottom > cactusTop &&
        dinoTop < GROUND_Y
    );
}

// ==========================
// RESET GAME
// ==========================

void resetGame() {

    dinoY = GROUND_Y - 10;
    velocity = 0;

    cactusX = SCREEN_WIDTH + 20;

    score = 0;
    gameSpeed = 2;

    jumping = false;
    gameOver = false;
}

// ==========================
// START SCREEN
// ==========================

void drawStartScreen() {

    display.clearDisplay();

    display.setTextSize(2);
    display.setCursor(12, 10);
    display.println("DINO GAME");

    display.setTextSize(1);
    display.setCursor(20, 38);
    display.println("Press button to play");

    display.display();
}

// ==========================
// GAME OVER SCREEN
// ==========================

void drawGameOver() {

    display.clearDisplay();

    display.setTextSize(2);
    display.setCursor(15, 8);
    display.println("GAME OVER");

    display.setTextSize(1);

    display.setCursor(35, 32);
    display.print("Score: ");
    display.println(score);

    display.setCursor(10, 50);
    display.println("Press to restart");

    display.display();
}

// ==========================
// SETUP
// ==========================

void setup() {

    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    Wire.begin(OLED_SDA, OLED_SCL);

    if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
    )) {

        Serial.println("OLED initialization failed!");

        while (true) {
            delay(100);
        }
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    drawStartScreen();

    randomSeed(analogRead(34));
}

// ==========================
// MAIN GAME LOOP
// ==========================

void loop() {

    if (millis() - lastFrame < FRAME_DELAY) {
        return;
    }

    lastFrame = millis();

    bool pressed = buttonPressed();

    // Start game
    if (!started) {

        if (pressed) {
            started = true;
            resetGame();
        }

        return;
    }

    // Restart after game over
    if (gameOver) {

        if (pressed) {
            resetGame();
        }

        drawGameOver();

        return;
    }

    // Jump
    if (pressed && !jumping) {

        velocity = JUMP_FORCE;
        jumping = true;
    }

    // Dinosaur physics
    dinoY += velocity;
    velocity += GRAVITY;

    // Ground collision
    if (dinoY >= GROUND_Y - 10) {

        dinoY = GROUND_Y - 10;

        velocity = 0;
        jumping = false;
    }

    // Move cactus
    cactusX -= gameSpeed;

    // Spawn next cactus
    if (cactusX < -10) {

        cactusX = SCREEN_WIDTH + random(15, 50);

        score++;
        if (score % 3 == 0 && gameSpeed < 4) {
        gameSpeed++;}
    }

    // Check collision
    if (collision()) {

        gameOver = true;

        drawGameOver();

        return;
    }

    // ======================
    // DRAW EVERYTHING
    // ======================

    display.clearDisplay();

    // Ground
    display.drawLine(
        0,
        GROUND_Y,
        SCREEN_WIDTH,
        GROUND_Y,
        SSD1306_WHITE
    );

    // Dinosaur
    drawDino(DINO_X, (int)dinoY);

    // Cactus
    drawCactus(cactusX);

    // Score
    display.setTextSize(1);
    display.setCursor(88, 5);

    display.print("S:");
    display.print(score);

    display.display();
}
