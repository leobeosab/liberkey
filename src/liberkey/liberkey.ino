#include<Wire.h>
#include <bluefruit.h>

BLEDis bledis;
BLEHidAdafruit blehid;

bool hasKeyPressed = false;

const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

int minVal = 265;
int maxVal = 402;

double x;
double y;
double z;

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 200; // .2 second update interval

uint8_t sKeycode[6] = {HID_KEY_S, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE };
uint8_t wKeycode[6] = {HID_KEY_W, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE };
uint8_t aKeycode[6] = {HID_KEY_A, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE };
uint8_t dKeycode[6] = {HID_KEY_D, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE };
uint8_t noKeycode[6] = {HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_NONE };

void setup() {
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  // Configure and Start Device Information Service
  bledis.setManufacturer("Wisearino");
  bledis.setModel("Liberkey");
  bledis.begin();

  blehid.begin();

  // Set callback for set LED from central
  blehid.setKeyboardLedCallback(set_keyboard_led);

  // Set up and start advertising
  startAdv();

  pinMode(10, INPUT_PULLUP); 
  pinMode(9, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);

  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
}

bool isActive() {
  return y >= 220;
}

void loop() {
  if (millis() - lastUpdateTime >= updateInterval) {
    updateMeasurements(); // Update measurements every second

    if (isActive()) {
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
      blehid.keyboardReport(1, noKeycode);
    } else {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
      blehid.keyboardReport(0, noKeycode);
    }
  }

  if (digitalRead(10) == LOW) { // Check if pin 10 is pressed s
    blehid.keyboardReport(1, sKeycode);
    delay(4);
    while (digitalRead(9) == LOW) {} // Wait until pin 10 is released
    blehid.keyboardReport(1, noKeycode);
    delay(100); // Delay to avoid multiple prints due to bouncing
  }

  if (digitalRead(9) == LOW) { // Check if pin 10 is pressed w
    blehid.keyboardReport(1, wKeycode);
    delay(4);
    while (digitalRead(9) == LOW) {} // Wait until pin 10 is released
    blehid.keyboardReport(1, noKeycode);
    delay(100); // Delay to avoid multiple prints due to bouncing
  }

  if (digitalRead(1) == LOW) { // Check if pin 10 is pressed d
    blehid.keyboardReport(1, dKeycode);
    delay(4);
    while (digitalRead(9) == LOW) {} // Wait until pin 10 is released
    blehid.keyboardReport(1, noKeycode);
    delay(100); // Delay to avoid multiple prints due to bouncing
  }
    
  if (digitalRead(7) == LOW) { // Check if pin 10 is pressed a
    blehid.keyboardReport(1, aKeycode);
    delay(5);
    while (digitalRead(9) == LOW) {} // Wait until pin 10 is released
    blehid.keyboardReport(1, noKeycode);
    delay(100); // Delay to avoid multiple prints due to bouncing
  }
}

void updateMeasurements() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  int xAng = map(AcX, minVal, maxVal, -90, 90);
  int yAng = map(AcY, minVal, maxVal, -90, 90);
  int zAng = map(AcZ, minVal, maxVal, -90, 90);

  x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);

  lastUpdateTime = millis();
}


/**
 * Callback invoked when received Set LED from central.
 * Must be set previously with setKeyboardLedCallback()
 *
 * The LED bit map is as follows: (also defined by KEYBOARD_LED_* )
 *    Kana (4) | Compose (3) | ScrollLock (2) | CapsLock (1) | Numlock (0)
 */
void set_keyboard_led(uint16_t conn_handle, uint8_t led_bitmap)
{
  (void) conn_handle;
  
  // light up Red Led if any bits is set
  if ( led_bitmap )
  {
    ledOn( LED_RED );
  }
  else
  {
    ledOff( LED_RED );
  }
}

  Serial.println(x);

  Serial.print("AngleY= ");
  Serial.println(y);

  Serial.print("AngleZ= ");
  Serial.println(z);
  Serial.println("-----------------------------------------");
}

void startAdv(void)
{  
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
  
  Bluefruit.Advertising.addService(blehid);

  Bluefruit.Advertising.addName();
  
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}
