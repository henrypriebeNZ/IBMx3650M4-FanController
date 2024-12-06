#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Define I2C pins for ESP32
#define SDA_PIN 21
#define SCL_PIN 22

// Define rotary encoder pins
#define CLK_PIN 27
#define DT_PIN 26
#define SW_PIN 25

// FAN 1
#define PWM_PIN_1 17
#define PWM_PIN_2 16
#define PWM_CHANNEL_1 0 // Channel for PWM_PIN_1
#define PWM_CHANNEL_2 1 // Channel for PWM_PIN_2

// FAN 2
#define PWM_PIN_3 18
#define PWM_PIN_4 19
#define PWM_CHANNEL_3 2 // Channel for PWM_PIN_3
#define PWM_CHANNEL_4 3 // Channel for PWM_PIN_4

// FAN 3
#define PWM_PIN_5 32
#define PWM_PIN_6 33
#define PWM_CHANNEL_5 4 // Channel for PWM_PIN_5
#define PWM_CHANNEL_6 5 // Channel for PWM_PIN_6

// FAN 4
#define PWM_PIN_7 12
#define PWM_PIN_8 14
#define PWM_CHANNEL_7 6 // Channel for PWM_PIN_7
#define PWM_CHANNEL_8 7 // Channel for PWM_PIN_8

const int pwmChannels[] = {PWM_CHANNEL_1, PWM_CHANNEL_2, PWM_CHANNEL_3, PWM_CHANNEL_4,
                           PWM_CHANNEL_5, PWM_CHANNEL_6, PWM_CHANNEL_7, PWM_CHANNEL_8};


#define PWM_FREQUENCY 25000 // 25 kHz
#define PWM_RESOLUTION 8    // 8-bit resolution (0-255)

// Define fan speed variables
volatile int encoderValue = 100; // Initial fan speed
volatile bool valueChanged = true; // Flag to update display
bool initRun = 1;

// Rotary encoder state tracking
int lastClkState;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // Debounce delay in ms


unsigned long _lastIncReadTime = micros();
unsigned long _lastDecReadTime = micros();
int _pauseLength = 25000;
int _fastIncrement = 10;

volatile int counter = 100;

//Control
bool buttonPressed = 0;
bool controlUnlocked = 0;

// Initialize OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Bitmap logo
const unsigned char chatrLogoWhite[] PROGMEM = {
0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x80, 0x00, 0x00,
0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x00,
0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00,
0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf8, 0xff, 0xf0, 0x00,
0x00, 0x1f, 0xff, 0xff, 0xbf, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0xff, 0xfb, 0xff, 0xff, 0xfe, 0x00,
0x00, 0x7f, 0xff, 0xc0, 0x00, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xff, 0xdf, 0xfc, 0xff, 0xff, 0x00,
0x01, 0xff, 0xff, 0xdf, 0xcc, 0xff, 0xff, 0x80, 0x03, 0xff, 0xff, 0xd9, 0x84, 0xff, 0xff, 0xc0,
0x07, 0xff, 0xff, 0xdf, 0x84, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xd9, 0x84, 0xff, 0xff, 0xe0,
0x0f, 0xff, 0xff, 0xdf, 0xcc, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xc0, 0x00, 0xff, 0xff, 0xf0,
0x1f, 0xff, 0xff, 0xc0, 0x00, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0xff, 0xf3, 0xff, 0xff, 0xff, 0xf8,
0x3f, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xe1, 0xf3, 0xff, 0xff, 0xfc, 0x0f, 0xfc,
0x3f, 0xc0, 0xf3, 0xff, 0xff, 0xbc, 0x07, 0xfc, 0x7f, 0x8c, 0x73, 0xff, 0xff, 0x3c, 0x43, 0xfe,
0x7f, 0x8c, 0x73, 0xff, 0xff, 0x3c, 0x73, 0xfe, 0x7f, 0x8c, 0x72, 0x3e, 0x0e, 0x0c, 0x73, 0xfe,
0x7f, 0x8f, 0xf0, 0x1c, 0x06, 0x0c, 0x73, 0xfe, 0xff, 0x8f, 0xf1, 0x9c, 0xc7, 0x3c, 0x63, 0xff,
0xff, 0x8f, 0xf3, 0x9c, 0xe7, 0x3c, 0x03, 0xff, 0xff, 0x8f, 0xf3, 0x9f, 0xc7, 0x3c, 0x07, 0xff,
0xff, 0x8f, 0xf3, 0x9f, 0x07, 0x3c, 0x47, 0xff, 0xff, 0x8c, 0x73, 0x9e, 0x27, 0x3c, 0x67, 0xff,
0xff, 0x8c, 0x73, 0x9c, 0xe7, 0x3c, 0x67, 0xff, 0xff, 0x8c, 0x73, 0x9c, 0xe7, 0x3c, 0x63, 0xff,
0xff, 0x8c, 0x73, 0x9c, 0xc7, 0x1c, 0x63, 0xff, 0xff, 0xc0, 0xf3, 0x9c, 0x07, 0x0c, 0x73, 0xff,
0xff, 0xe1, 0xf3, 0x9c, 0x27, 0x8c, 0x71, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfe,
0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,
0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xc9, 0x2a, 0x2a, 0x4d, 0x29, 0x53, 0xfc,
0x1f, 0xea, 0x2a, 0x2a, 0x57, 0xa9, 0x17, 0xf8, 0x1f, 0xfa, 0x08, 0x28, 0x5e, 0xa9, 0x13, 0xf8,
0x0f, 0xea, 0x48, 0x28, 0x50, 0xa9, 0x5b, 0xf0, 0x0f, 0xc9, 0x49, 0x4a, 0x4a, 0xa9, 0x53, 0xf0,
0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0,
0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,
0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00,
0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00,
0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00,
0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
0x00, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00,
0x00, 0x00, 0x01, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00
};

void update_PWMs(int scaledDutyCycle);
void setupPWMChannel(int channel, int pin);
void setupPin(int pin);
void updateDisplay(int encoderValue, bool unlocked);
void updateCounter(int changeValue);
void read_encoder();





void setup() {
  
  // Initialize I2C communication
  Wire.begin(SDA_PIN, SCL_PIN);

// Configure PWM channels
setupPWMChannel(PWM_CHANNEL_1, PWM_PIN_1);
setupPWMChannel(PWM_CHANNEL_2, PWM_PIN_2);
setupPWMChannel(PWM_CHANNEL_3, PWM_PIN_3);
setupPWMChannel(PWM_CHANNEL_4, PWM_PIN_4);
setupPWMChannel(PWM_CHANNEL_5, PWM_PIN_5);
setupPWMChannel(PWM_CHANNEL_6, PWM_PIN_6);
setupPWMChannel(PWM_CHANNEL_7, PWM_PIN_7);
setupPWMChannel(PWM_CHANNEL_8, PWM_PIN_8);


setupPin(PWM_PIN_1);
setupPin(PWM_PIN_2);
setupPin(PWM_PIN_3);
setupPin(PWM_PIN_4);
setupPin(PWM_PIN_5);
setupPin(PWM_PIN_6);
setupPin(PWM_PIN_7);
setupPin(PWM_PIN_8);



  // Initialize encoder pins
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(CLK_PIN), read_encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DT_PIN), read_encoder, CHANGE);

  // Read initial state of CLK pin
  lastClkState = digitalRead(CLK_PIN);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1); // Halt if display fails to initialize
  }

  // Set initial PWM duty cycle
  int initialDutyCycle = map(encoderValue, 20, 198, 0, 254);
   ledcWrite(PWM_CHANNEL_1, initialDutyCycle);
   ledcWrite(PWM_CHANNEL_2, initialDutyCycle);
   ledcWrite(PWM_CHANNEL_3, initialDutyCycle);
   ledcWrite(PWM_CHANNEL_4, initialDutyCycle);
   ledcWrite(PWM_CHANNEL_5, initialDutyCycle);
   ledcWrite(PWM_CHANNEL_6, initialDutyCycle);
   ledcWrite(PWM_CHANNEL_7, initialDutyCycle);
   ledcWrite(PWM_CHANNEL_8, initialDutyCycle);

  // Initial display setup
  display.clearDisplay();
  display.display();

    ledcSetup(PWM_CHANNEL_1, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN_1, PWM_CHANNEL_1);
    ledcSetup(PWM_CHANNEL_2, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN_2, PWM_CHANNEL_2);
    ledcSetup(PWM_CHANNEL_3, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN_3, PWM_CHANNEL_3);
    ledcSetup(PWM_CHANNEL_4, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN_4, PWM_CHANNEL_4);
    ledcSetup(PWM_CHANNEL_5, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN_5, PWM_CHANNEL_5);
    ledcSetup(PWM_CHANNEL_6, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN_6, PWM_CHANNEL_6);
    ledcSetup(PWM_CHANNEL_7, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN_7, PWM_CHANNEL_7);
    ledcSetup(PWM_CHANNEL_8, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN_8, PWM_CHANNEL_8);
}

void setupPWMChannel(int channel, int pin) {
    ledcSetup(channel, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(pin, channel);
}

void setupPin(int pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}


void update_PWMs(int scaledDutyCycle) {
    for (int i = 0; i < 8; i++) {
        ledcWrite(pwmChannels[i], scaledDutyCycle);
    }
}

void updateDisplay(int encoderValue, bool unlocked) {
    display.clearDisplay(); // Clear the display buffer

    // Draw bitmap at the top center
    display.drawBitmap((SCREEN_WIDTH - 16) / 2, 0, chatrLogoWhite, 64, 64, WHITE);

    // Display lock/unlock status
    display.setTextSize(1); // Small text size
    display.setTextColor(WHITE); // Set text color to white
    display.setCursor(0, 0); // Set cursor position
    display.print(unlocked ? F("Unlocked") : F("Locked"));

    // Display fan speed
    display.setCursor(0, 20); // Position cursor below bitmap
    display.print(F("Fan"));
    display.println();
    display.print(F("Speed: "));

    display.setTextSize(3); // Larger text size for speed
    display.setCursor(0, 40); // Position cursor for speed
    if (encoderValue <= 41) {
        display.print(F("MIN")); // Display "MIN" if speed is too low
    } else if (encoderValue > 199) {
        display.print(F("MAX")); // Display "MAX" if speed is too high
    } else {
        display.print(encoderValue / 2); // Display percentage (scaled speed)
        display.print(F("%"));
    }

    display.display(); // Push buffer to display
}

void updateCounter(int changeValue) {
    if ((micros() - (changeValue > 0 ? _lastIncReadTime : _lastDecReadTime)) < _pauseLength) {
        changeValue *= _fastIncrement;
    }
    if (changeValue > 0) _lastIncReadTime = micros();
    else _lastDecReadTime = micros();

    counter = constrain(counter + changeValue, 40, 200);
}



void loop() {
  // Read current state of CLK pin
  int currentClkState = digitalRead(CLK_PIN);



  //FAN CONTROL
  int fan_minimum = 55;

// Calculate duty cycles for both PWM channels
int scaledDutyCycle = map(encoderValue, 40, 200, fan_minimum, 255);  // For PWM_PIN_1

// Update PWM duty cycles

  update_PWMs(scaledDutyCycle);




// CONTROLLER
  // Check if button is pressed
if (digitalRead(SW_PIN) == LOW && !buttonPressed) {
    buttonPressed = true; // Flag to prevent multiple triggers
    controlUnlocked = !controlUnlocked;
    valueChanged = true;
    delay(50);            // Short delay for debounce
} else if (digitalRead(SW_PIN) == HIGH) {
    buttonPressed = false; // Reset flag when button is released
    valueChanged = false;
    display.clearDisplay();
}

if (controlUnlocked == 1 || initRun == 1) {
  encoderValue = constrain(counter, 40, 200); // Constrain counter to valid range
  valueChanged = true; // Mark the value as changed

  // Update last CLK state
  lastClkState = currentClkState;
  }

  updateDisplay(encoderValue, controlUnlocked);

  initRun = 0;
}


void read_encoder() {
  if (controlUnlocked == 1){

  if (counter >= 201){
    counter = 200;
  }

  if (counter <= 39){
    counter = 40;
  }
  // Encoder interrupt routine for both pins. Updates counter
  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value
  static const int8_t enc_states[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0}; // Lookup table

  old_AB <<= 2; // Remember previous state

  if (digitalRead(CLK_PIN)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(DT_PIN)) old_AB |= 0x01; // Add current state of pin B

  encval += enc_states[(old_AB & 0x0F)];

  // Update counter if encoder has rotated a full indent (at least 4 steps)
  if (encval > 3) {
    updateCounter(1);
    encval = 0;
  } else if (encval < -3) {
    updateCounter(-1);
    encval = 0;
  }
  }
}
