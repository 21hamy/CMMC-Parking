/*
  NOTE:
  ======
  Only RTC IO can be used as a source for external wake
  source. They are pins: 0,2,4,12-15,25-27,32-39.
*/

// RX TX pins
#define RXD2 17
#define TXD2 18

// Wake up pin
#define BUTTON_PIN_BITMASK 0x8004 // 2 and 15

RTC_DATA_ATTR int bootCount = 0;

// Wake up time set
#define uS_TO_S_FACTOR 500000
#define Time_Sleep_S 5

///// Ultrasonic /////////////////
#define echoPin 26
#define trigPin 25
long duration, distance;
int count = 0, countLoop = 0;
//////////////////////////////////

///// Moter //////////////////////
int se_a = 2;
int se_a_s = 2;
int output_A = 0;
//////////////////////////////////

char readChar;
float VP;
int triggeredPin = 0;
unsigned long previousMillis = 0;

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
  wait_command();
}

void print_GPIO_wake_up() {
  triggeredPin = 0;
  int GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  triggeredPin = (log(GPIO_reason)) / log(2);
  Serial.print("GPIO that triggered to wake up: GPIO ");
  Serial.println((log(GPIO_reason)) / log(2), 0);
}

void setup() {
  //Serial config
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  //Ultrasonic pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //ADC pin
  pinMode(36, INPUT);

  //Buzzer pin
  pinMode(22, OUTPUT);

  //Trigger WUL pin
  pinMode(27, OUTPUT);

  ////////////////////////////// Moter Control ////////////////////////////////
  //GPIO14 Status                              ////////////////////////////////
  pinMode(14, INPUT);                          ////////////////////////////////
  //IR Sensor pins                             ////////////////////////////////
  pinMode(32, INPUT_PULLUP);                   ////////////////////////////////
  pinMode(33, INPUT_PULLUP);                   ////////////////////////////////
  //Moter pins                                 ////////////////////////////////
  pinMode(19, OUTPUT);                         ////////////////////////////////
  pinMode(21, OUTPUT);                         ////////////////////////////////
  //GPIO23 Trigger moter                       ////////////////////////////////
  pinMode(23, OUTPUT);                         ////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  //Take some time to open up the Serial Monitor
  delay(1000);

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  //Print the Pins that triggered to wake up.
  print_GPIO_wake_up();

  //Multi pins
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  //Wake up by timer
  //esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * Time_Sleep_S);
}

void pushData () {
  Serial2.println(VP);
}

void getData () {
  if (Serial2.available() > 0) {
    readChar = Serial2.read();
    Serial.println("Wait commnad...");
    if (readChar == 'V') {
      pushData();
    }

    if (readChar == 'S') {
      Goto_sleep_now();
    }
  }
}

void loop() {
  Serial.println("Loop ! ! ! ");

  //Multi pins
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  wait_command();
  countLoop++;
  delay(100);

  if (triggeredPin == 2) {
    countLoop = 0;
  } else if (triggeredPin == 15) {
    countLoop = 0;
  }

  if (countLoop > 5) {
    Goto_sleep_now();
  }
}

void wait_command () {
  if (triggeredPin == 2) {
    getData();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 500) {
      previousMillis = currentMillis;
      digitalWrite(22, HIGH);
      Serial.println("alert ");
    }
    if (currentMillis - previousMillis >= 1000) {
      previousMillis = currentMillis;
      digitalWrite(22, LOW);
    }
  } else if (triggeredPin == 15) {
    getVoltage();
    getData();
    Moter_control();

    Serial.println(count);
    count++;
    if (count > 100) {
      count = 0;
      Goto_sleep_now();
    }
  }
}

void Moter_control () {
  getWUL();
  int GPIO14 = digitalRead(14);
  int IR_1 = digitalRead(32);
  int IR_2 = digitalRead(33);

  // Trigger GPIO 23
  digitalWrite(23, HIGH);
  //  Serial.print(aIR_1);
  //  Serial.println(IR_2);

  if (GPIO14 == 1 && distance > 100) {
    se_a_s = 0;
  } else if (GPIO14 == 1 && distance < 100) {
    se_a_s = 2;
  } else if (GPIO14 == 0) {
    se_a_s = 2;
  }

  //  if (Serial.available() > 0) {    // is a character available?
  //    readSerial = Serial.read();
  //    digitalWrite(23, HIGH);
  //    if (readSerial == 'U') {
  //      se_a_s = 0;
  //    }
  //    if (readSerial == 'D') {
  //      se_a_s = 2;
  //    }
  //  }

  if (IR_1 == 0 && IR_2 == 0) {
    se_a = 0;
  } else if (IR_1 == 0 && IR_2 == 1) {
    se_a = 1;
  } else if (IR_1 == 1 && IR_2 == 0) {
    se_a = 1;
  } else if (IR_1 == 1 && IR_2 == 1) {
    se_a = 2;
  }
  output_A = se_a - se_a_s;

  if (output_A < 0) {
    digitalWrite(19, LOW);
    digitalWrite(21, HIGH);
    count = 0;
  } else if (output_A > 0) {
    digitalWrite(19, HIGH);
    digitalWrite(21, LOW);
    count = 0;
  } else {
    digitalWrite(19, LOW);
    digitalWrite(21, LOW);
    digitalWrite(23, LOW);
    Serial.print(se_a);
    Serial.print(",");
    Serial.println(se_a_s);
  }
}

void getVoltage () {
  VP = analogRead(36);
  VP = (VP / 4096) * 6.6;
  Serial.print("Your battery voltage : ");
  Serial.print(VP);
  Serial.println("v");
}

void getWUL () {
  digitalWrite(27, HIGH);

  digitalWrite(trigPin, LOW);
  delayMicroseconds(10);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(50);

  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  //Serial.println(duration);
  distance = duration / 58.2;
  if (distance == 0) {

  }
  Serial.print("Measuring distance : ");
  Serial.print(distance);
  Serial.println(" cm");
}

void Goto_sleep_now () {
  //Go to sleep now
  Serial.println("Going to sleep now");
  delay(1000);
  esp_deep_sleep_start();
}
