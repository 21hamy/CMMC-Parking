#define RXD2 17
#define TXD2 18
float convert = 0;
char DATA;
String voltage = " ";
void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.begin(115200);
  //Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial Txd is on pin: " + String(TX));
  Serial.println("Serial Rxd is on pin: " + String(RX));
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
}

void loop() { //Choose Serial1 or Serial2 as required
  char readChar;
  DATA = ' ';
  if (Serial.available() > 0) {
    readChar = Serial.read();
    if (readChar != '\n') {
      // a character of the string was received
      DATA = readChar;
    }
    Serial2.println(DATA);
    Serial.println(DATA);
  }

  char readINPUT;
  if (Serial2.available() > 0) {
    readINPUT = Serial2.read();
    //    Serial.println(readINPUT);
    if (readINPUT == '.') {
      voltage += readINPUT;
    }
    if (readINPUT == '1') {
      voltage += readINPUT;
    }
    if (readINPUT == '2') {
      voltage += readINPUT;
    }
    if (readINPUT == '3') {
      voltage += readINPUT;
    }
    if (readINPUT == '4') {
      voltage += readINPUT;
    }
    if (readINPUT == '5') {
      voltage += readINPUT;
    }
    if (readINPUT == '6') {
      voltage += readINPUT;
    }
    if (readINPUT == '7') {
      voltage += readINPUT;
    }
    if (readINPUT == '8') {
      voltage += readINPUT;
    }
    if (readINPUT == '9') {
      voltage += readINPUT;
    }
    if (readINPUT == '0') {
      voltage += readINPUT;
    }
    convert = voltage.toFloat();
  }
  Serial.print("Read voltage : ");
  Serial.print(convert);
  Serial.println(" v");

  if (readChar == 'A') {
    digitalWrite(12, HIGH);
    digitalWrite(13, HIGH);
    delay(200);
  }
  if (readChar == 'Z') {
    digitalWrite(12, HIGH);
    digitalWrite(13, LOW);
    delay(200);
  }

  digitalWrite(12, LOW);

}
