#include <WiFi.h>
#include <WiFiUdp.h>

const char* s = "daawarNOTFOUND";
const char* p = "7866aooo";

WiFiUDP u;
const int pt = 8080;
char pkt[255];

const int ea = 14;
const int i1 = 27;
const int i2 = 26;
const int eb = 32;
const int i3 = 25;
const int i4 = 33;
const int rp = 13;

int hs = 200;
unsigned long lbt = 0;
bool ext = false;

void sm() {
  digitalWrite(i1, LOW); digitalWrite(i2, LOW);
  digitalWrite(i3, LOW); digitalWrite(i4, LOW);
  analogWrite(ea, 0); analogWrite(eb, 0);
}

void mf() {
  analogWrite(ea, hs); analogWrite(eb, hs);
  digitalWrite(i1, HIGH); digitalWrite(i2, LOW);
  digitalWrite(i3, HIGH); digitalWrite(i4, LOW);
}

void mb() {
  analogWrite(ea, hs); analogWrite(eb, hs);
  digitalWrite(i1, LOW); digitalWrite(i2, HIGH);
  digitalWrite(i3, LOW); digitalWrite(i4, HIGH);
}

void tl() {
  analogWrite(ea, 125); 
  analogWrite(eb, 125); 
  digitalWrite(i1, LOW); digitalWrite(i2, HIGH);
  digitalWrite(i3, HIGH); digitalWrite(i4, LOW);
}

void tr() {
  analogWrite(ea, 125); 
  analogWrite(eb, 125); 
  digitalWrite(i1, HIGH); digitalWrite(i2, LOW);
  digitalWrite(i3, LOW); digitalWrite(i4, HIGH);
}

void sc() {
  analogWrite(ea, 125); 
  analogWrite(eb, 125); 
  digitalWrite(i1, HIGH); digitalWrite(i2, LOW);
  digitalWrite(i3, LOW); digitalWrite(i4, HIGH);
}

void he() {
  sm();
  unsigned long cm = millis();
  unsigned long el = cm - lbt;

  if (el < 500) {
    digitalWrite(rp, LOW);
  } else if (el < 1500) {
    digitalWrite(rp, HIGH);
  } else {
    lbt = cm;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ea, OUTPUT); pinMode(i1, OUTPUT); pinMode(i2, OUTPUT);
  pinMode(eb, OUTPUT); pinMode(i3, OUTPUT); pinMode(i4, OUTPUT);
  pinMode(rp, OUTPUT);
  digitalWrite(rp, HIGH);
  sm();

  WiFi.begin(s, p);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println(WiFi.localIP());
  u.begin(pt);
}

void loop() {
  int sz = u.parsePacket();
  
  if (sz) {
    int l = u.read(pkt, 255);
    if (l > 0) pkt[l] = 0;
    char c = pkt[0];

    if (c != 'x') {
      ext = false;
      digitalWrite(rp, HIGH);
    }

    if (c == 'f') { 
      hs = 180;
      mf();
    }
    else if (c == 'b') mb();
    else if (c == 'l') tl();
    else if (c == 'r') tr();
    else if (c == 's') sm();
    else if (c == 'x') {
      if (!ext) {
        ext = true;
        lbt = millis(); 
      }
    }
    else if (c == 'c') sc(); 
  }

  if (ext) {
    he();
  }
}