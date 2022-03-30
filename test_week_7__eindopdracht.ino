#define BLYNK_PRINT Serial

#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <MFRC522.h>


//hier word pinne gedefind
#define LIGHT_SENSOR_PIN 35
#define magneet 25
#define drk1 26
#define motion 27
#define SS_PIN 21
#define RST_PIN 22
#define deurSlot 4
#define ledMotion 5
char auth[] = "STMZ47u-7aG_BiDovWO8J9iWR4D4KI7W";
char ssid[] = "telenet-1036B";
char pass[] = "aAF3vwajKsVT";

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.


//status onthouden
int alarmAan = 0;
int deur = 0;
int drkGSM;


WidgetLED led1(V1);

void setup()
{
  Serial.begin(9600);   // Initiate a serial communication
  Blynk.begin(auth, ssid, pass, "server.wyns.it", 8081);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  pinMode(motion, INPUT);
  pinMode(drk1, INPUT_PULLUP);
  pinMode(magneet, INPUT_PULLUP);
  pinMode(deurSlot, OUTPUT);
  pinMode(ledMotion, OUTPUT);

}
BLYNK_WRITE(V5) // this command is listening when something is written to V1
{
  int PinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  Serial.print(PinValue);
  drkGSM = PinValue;
}


void loop()
{
  Blynk.run();
  if (digitalRead(drk1) == LOW || drkGSM == 1 )
  {
    Serial.print("drk1");
    while (digitalRead(drk1) == LOW)
    {
      delay(100);
    }
    delay(1000 * 10);
    alarmAan = 1;
    Serial.print("alarm actief");
    digitalWrite(deurSlot, HIGH);
    led1.on();
  }


    while (alarmAan == 1)
    {
      Blynk.run();
      int analogValue = analogRead(LIGHT_SENSOR_PIN);
      Blynk.virtualWrite(V6, analogValue);
      if (digitalRead(magneet) == HIGH && deur == 0)
      {
        Serial.println("deur is geopent");
        Blynk.notify("alarm deur geopent");
        deur = 1;
        delay(1000);
      }
      if (digitalRead(magneet) == LOW && deur == 1)
      {
        Serial.println("deur is dicht");
        deur = 0;
        delay(1000);
      }

      if (digitalRead(motion) == HIGH)
      {
        Serial.println("motion detected");
        Serial.println(analogValue);
        Blynk.notify("alarm motion detected");
        if (analogValue <= 1000)
        {
          digitalWrite(ledMotion, HIGH);
          delay(5000);
          digitalWrite(ledMotion, LOW);
        }

      }

      // Look for new cards
      if ( ! mfrc522.PICC_IsNewCardPresent())
      {
        return;
      }
      // Select one of the cards
      if ( ! mfrc522.PICC_ReadCardSerial())
      {
        return;
      }

      //Show UID on serial monitor
      Serial.print("UID tag :");
      String content = "";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++)
      {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      Serial.println();
      Serial.print("Message : ");
      content.toUpperCase();
      if (content.substring(1) == "93 A9 17 13" || content.substring(1) == "F3 8E BA 0D") // change here the UID of the card/cards that you want to give access
      {
        Serial.println("Authorized access");
        Serial.println();
        alarmAan = 0;
        led1.off();
        digitalWrite(deurSlot, LOW);
        Blynk.notify("access granted");
        delay(1000);
      }

      else   {
        Serial.println(" Access denied");
        Blynk.notify("alarm Access denied");
        delay(1000);
      }

    }
  
}
