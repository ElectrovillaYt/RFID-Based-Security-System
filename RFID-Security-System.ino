#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial RYRR10S_Serial(2, 3);  // RX with D3 and TX with D2

uint8_t echo_command[1] = { 0x55 };  //--> The ECHO command (Varifying Serial Communication)

uint8_t init_command[5][6] = {  //Initialisation commands
  { 0x09, 0x04, 0x68, 0x01, 0x07, 0x10 },
  { 0x09, 0x04, 0x68, 0x01, 0x07, 0x00 },
  { 0x02, 0x02, 0x02, 0x00, 0x00, 0x00 },
  { 0x09, 0x04, 0x3A, 0x00, 0x58, 0x04 },
  { 0x09, 0x04, 0x68, 0x01, 0x01, 0xD3 }
};

//Detect Command
uint8_t detect_cmd_iso14443_RAA[4] = { 0x04, 0x02, 0x26, 0x07 };        // Command to REQAreply ATQA.
uint8_t detect_cmd_iso14443_AC1[5] = { 0x04, 0x03, 0x93, 0x20, 0x08 };  // Command to ISO14443-A ANTICOL 1.

String key = "b3c6c9be"; // Your Rfid Key

uint8_t received_buf_pos = 0;
uint8_t response_byte;
uint8_t data_len;
String tag_id = "";
char Byte_In_Hex[3];
bool tag_Detected = false;
const int out = 4;

const unsigned int lock_timing = 10000; //Set lock timing in ms according to your need!


void show_serial_data() {
  while (RYRR10S_Serial.available() != 0) Serial.print(RYRR10S_Serial.read(), HEX);
  Serial.println("");
}

void init_RYRR10S() {
  Serial.println();
  Serial.print("Echo command | Respone : ");
  RYRR10S_Serial.write(echo_command, 1);
  delay(50);
  show_serial_data();
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 6; j++) {
      RYRR10S_Serial.write(init_command[i][j]);
      delay(50);
    }
    show_serial_data();
  }
}

void Read_Tag() {
  uint8_t received_char;
  while (RYRR10S_Serial.available() != 0) {
    received_char = char(RYRR10S_Serial.read());
    if (received_buf_pos == 0) response_byte = received_char;
    else if (received_buf_pos == 1) data_len = received_char;
    else if (received_buf_pos >= 2 and received_buf_pos < 6) {
      sprintf(Byte_In_Hex, "%x", received_char);
      tag_id += Byte_In_Hex;  // adding to a string.
    }
    received_buf_pos++;
  }
}

void Detect_Tag() {
  received_buf_pos = 0;
  tag_id = "";
  response_byte = 0;

  RYRR10S_Serial.write(detect_cmd_iso14443_RAA, 4);
  delay(200);
  while (RYRR10S_Serial.available() != 0) {
    RYRR10S_Serial.read();
  }
  RYRR10S_Serial.write(detect_cmd_iso14443_AC1, 5);
  delay(200);

  if (RYRR10S_Serial.available()) {
    Read_Tag();
  }
}

void setup() {
  Serial.begin(9600);
  RYRR10S_Serial.begin(57600);
  lcd.begin();
  pinMode(13, OUTPUT);
  pinMode(out, OUTPUT);
  delay(500);
  init_RYRR10S();
  lcd.clear();
}

void Verify_card() {
  if (tag_id == key) {
    lcd.setCursor(0, 0);
    lcd.print("Access Granted!!");
    digitalWrite(out, 1);
    delay(lock_timing);
    digitalWrite(out, 0);
  } else if (tag_id != key && tag_id != "") {
    digitalWrite(out, 0);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Denied!!!");
  } else {
    digitalWrite(out, 0);
    lcd.clear();
    lcd.print(" Scan your card");
  }
}

void loop() {
  Serial.println();
  Serial.println("Scanning card...");

  Detect_Tag();

  if (tag_Detected == true) {
    tag_Detected = false;
    Detect_Tag();
  }

  if (response_byte == 0x80 && tag_id != "") {
    Serial.print("Tag detected. Tag id : ");
    Serial.println(tag_id);
    Serial.println();
    tag_Detected = true;
    digitalWrite(13, 1);
    delay(1000);
  } else {
    Serial.println("No tag detected.");
    Serial.println("");
    digitalWrite(13, 0);
    delay(500);
  }
  Verify_card();
}