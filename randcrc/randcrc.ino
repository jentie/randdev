/*
  randcrc - random numbers via crc32 over analog input

  2024-04-26, jens
  
  based on: 
  https://excamera.com/sphinx/article-crc.html


  todo:
  * switching to higher baud rates does not work

*/


int analogPin = A3;


static const unsigned long crc_table[16] = {
  0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
  0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
  0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
  0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};


unsigned long day = 86400000;
unsigned long sec = 1000;


unsigned long period = sec;    // on & off time
unsigned long previousMillis;  // previous millis counter


bool single = false;
bool debug = false;
char format[] = "%08lX";
char buff[32];

unsigned long crc = 0x12345678;
unsigned long min = 0xffffffff;
unsigned long max = 0x00000000;


unsigned long crc32(unsigned long crc, char data) {
  byte i;

  i = crc ^ (data >> (0 * 4));
  crc = crc_table[i & 0x0f] ^ (crc >> 4);
  i = crc ^ (data >> (1 * 4));
  crc = crc_table[i & 0x0f] ^ (crc >> 4);
  return crc;
}


unsigned long crc_string(char *s) {
  unsigned long crc = ~0L;
  while (*s)
    crc = crc32(crc, *s++);
  crc = ~crc;
  return crc;
}


void changeSerial(int rate) {
  Serial.flush();  // sent last characters
  Serial.begin(rate);
  while (Serial.available())  // read garbage
    Serial.read();
}


void getRand() {

  int val = analogRead(analogPin);

  uint8_t data = (byte)(val & 0xFF);

  if (debug) {
    Serial.print(val, HEX);
    Serial.print("-");
    Serial.print(data, HEX);
    Serial.print(" -> ");
  }

  crc = crc32(crc, data);

  if (crc < min)
    min = crc;
  if (crc > max)
    max = crc;

  sprintf(buff, format, crc);
  Serial.println(buff);
}


void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(9600);
  Serial.println();

  previousMillis = millis();
}


void loop() {

  if (millis() - previousMillis >= period) {
    previousMillis = millis();

    getRand();

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));  // toggle LED
  }

  if (Serial.available()) {
    char c = Serial.read();

    switch (c) {
      case 'H':
      case 'h':
      case 'i':
        Serial.println("\nrandcrc - random numbers via crc32 over analog input");
        Serial.println("[i]nfo / [h]elp   [t]est   [d]ebug");
        Serial.println("output: [l]oop   [f]ast   [s]top   [r]equest / [?]");
        Serial.println("hex values with upper or lower case: [X] or [x]");
        Serial.println("baud rates: [1]9200, [2]8800, [3]8400, [5]7600, [9]600");
        break;

      case 'd':
        debug = !debug;
        break;

      case '1':
        changeSerial(19200);
        break;
      case '2':
        changeSerial(28800);
        break;
      case '3':
        changeSerial(38400);
        break;
      case '5':
        changeSerial(57600);
        break;
      case '9':
        changeSerial(9600);
        break;

      case 'r':
      case 'R':
      case '?':
        getRand();
        break;

      case 's':
        period = day;
        break;
      case 'l':
        period = sec;
        break;
      case 'f':
        period = 1;
        break;

      case 'x':
        format[4] = 'x';
        break;
      case 'X':
        format[4] = 'X';
        break;

      case 't':
        Serial.print("HELLO == 0xC1446436 == ");
        Serial.println(crc_string("HELLO"), HEX);

        Serial.print("min: ");
        sprintf(buff, format, min);
        Serial.print(buff);
        Serial.print("   max: ");
        sprintf(buff, format, max);
        Serial.println(buff);
        break;

      default:
        if (debug) {
          if (c > 15)
            Serial.print("0x");
          else
            Serial.print("0x0");
          Serial.print(c, HEX);
          Serial.println("?");
        }
    }  // switch
  }
}
