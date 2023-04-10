#include <MKRGSM.h>

const char PINNUMBER[] = "1234";

GSM gsmAccess;
GSM_SMS sms;

char senderNumber[20];
char phoneNumbers[10][20];
int phoneCount = 0;

int sensor_pin = A0;
int output_value;
//int soil=0;

unsigned long currentTime = 0;
unsigned long sendTime = 14400000; // send moisture every 4 hours

bool isSubscribed(char number[]) {
  for (int i = 0; i < phoneCount; i++) {
    if (strcmp(phoneNumbers[i], number) == 0) {
      return true;
    }
  }
  return false;
}

void subscribe(char number[]) {
  if (phoneCount < 10) {
    strcpy(phoneNumbers[phoneCount], number);
    phoneCount++;
    Serial.println("Subscribed " + String(number));
    sms.print("You have successfully subscribed to moisture updates. Thank you!");
  } else {
    Serial.println("Subscription failed - maximum number of subscribers reached.");
    sms.print("Sorry, the maximum number of subscribers has been reached.");
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  Serial.println("Moisture Sensor");

  bool connected = false;

  while (!connected) {
    if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("GSM initialized");
  Serial.println("Waiting for messages");

  delay(2000);
}

void loop() {
  currentTime = millis();

  if (sms.available()) {
    sms.remoteNumber(senderNumber, 20);
    String message = sms.readString();
    message.trim();

    Serial.print("Message received from: ");
    Serial.println(senderNumber);
    Serial.print("Message: ");
    Serial.println(message);

    if (message == "JOIN") {
      if (!isSubscribed(senderNumber)) {
        subscribe(senderNumber);
      } else {
        Serial.println("Already subscribed.");
        sms.print("You are already subscribed. Thank you!");
      }
    } else {
      Serial.println("Invalid message.");
      sms.print("Invalid message. Please send JOIN to subscribe. Thank you!");
    }

    sms.flush();
    Serial.println("Message deleted.");
  }

  if (currentTime - sendTime >= 0) {
    output_value = analogRead(sensor_pin);
    output_value = map(output_value, 550, 0, 0, 100);
    Serial.print("Moisture: ");
    Serial.print(output_value);
    Serial.println("%");

    for (int i = 0; i < phoneCount; i++) {
      sms.beginSMS(phoneNumbers[i]);
      sms.print("Moisture level: ");
      sms.print(output_value);
      sms.print("%");
      sms.endSMS();
      Serial.println("Message sent to " + String(phoneNumbers[i]));
    }

    sendTime = currentTime + 14400000; // send moisture every 4 hours
  }

  delay(1000);
}
