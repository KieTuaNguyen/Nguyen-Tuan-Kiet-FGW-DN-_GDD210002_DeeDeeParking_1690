//Include the library files
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <Thread.h>
#include <ThreadController.h>
#include <time.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <FirebaseESP8266.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define BLYNK_TEMPLATE_ID "TMPL6bE80JpYR"
#define BLYNK_TEMPLATE_NAME "Deedee Parking Project"
#define BLYNK_AUTH_TOKEN "O1FPsoO3-MV_iKuGK_oxhyjRXFWVOLTc"

Servo servo;

// Insert Firebase project API Key
#define API_KEY "AIzaSyDqKyGFVf3v9TySgTBRErRTzmyfZ0nv9iE"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "deedee-parking-project-default-rtdb.asia-southeast1.firebasedatabase.app"

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth fauth;
FirebaseConfig config;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "VT";        //Enter your WIFI name
char pass[] = "12341234";  //Enter your WIFI password

// define the pins for parking lot
const int irSensor1Pin = D1;
const int irSensor2Pin = D2;
const int irSensor3Pin = D3;
const int irSensor4Pin = D4;
const int irSensorGate0Pin = D5;
const int irSensorGate1Pin = D6;
const int servoPin = D0;
bool isParked[4] = { false, false, false, false };
String timeIn[4] = { "null", "null", "null", "null" };

// create and start the threads
Thread detectParking_thread = Thread();
Thread controlGate_thread = Thread();
ThreadController controller = ThreadController();

const long utcOffsetInSeconds = 25200;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

FirebaseData firebaseData;
String path = "/";
FirebaseJson json;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

//Define a function to handle switch servo in blynk
int old_blynk_cmd = 0;
BLYNK_WRITE(V0) {
  int switchState = param.asInt();
  if (switchState == 0 && switchState != old_blynk_cmd) {
    servo.write(0);
    old_blynk_cmd = switchState;
  } else if (switchState == 1 && switchState != old_blynk_cmd) {
    servo.write(180);
    old_blynk_cmd = switchState;
  }
}

// thread function to detect parking slot
void detectParkingSlot() {
  // read the values from each ir sensors
  int lot_1 = digitalRead(irSensor1Pin);
  int lot_2 = digitalRead(irSensor2Pin);
  int lot_3 = digitalRead(irSensor3Pin);
  int lot_4 = digitalRead(irSensor4Pin);

  // This mean a new car just entered
  if (lot_1 == LOW && isParked[0] == false) {
    Serial.println("A new car entered to slot 1");
    // save the time its entered and save to firebase
    // TIme
    timeClient.update();
    setTime(timeClient.getEpochTime());
    String timeString = String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second());
    Serial.println(timeString);
    isParked[0] = true;  // set the isParked to true
    timeIn[0] = timeString;
  } else if (lot_1 == LOW && isParked[0] == true) {
    // A new car is stay in the park, we already have save the time! Just print to the LCD that this slot is occupied
    Serial.println("Parking slot 1 is occupied!");
  } else if (lot_1 == HIGH && isParked[0] == true) {
    // A new car has just leave the parking slot!!! Save the leave time to firebase
    Serial.println("The car just leave the slot 1");
    isParked[0] = false;
    // TIme
    timeClient.update();
    setTime(timeClient.getEpochTime());
    String timeString = String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second());
    Serial.println(timeString);
    saveFirebase("1", timeIn[0], timeString);
  } else {
    // The parking slot now is empty!!!
    Serial.println("Parking slot 1 is empty");
  }

  // lot_2
  // This mean a new car just entered
  if (lot_2 == LOW && isParked[1] == false) {
    Serial.println("A new car entered to slot 2");
    // save the time its entered and save to firebase
    // TIme
    timeClient.update();
    setTime(timeClient.getEpochTime());
    String timeString = String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second());
    Serial.println(timeString);
    timeIn[1] = timeString;
    isParked[1] = true;  // set the isParked to true
  } else if (lot_2 == LOW && isParked[1] == true) {
    // A new car is stay in the park, we already have save the time! Just print to the LCD that this slot is occupied
    Serial.println("Parking slot 2 is occupied!");
  } else if (lot_2 == HIGH && isParked[1] == true) {
    // A new car has just leave the parking slot!!! Save the leave time to firebase
    Serial.println("The car just leave the slot 2");
    isParked[1] = false;
    // TIme
    timeClient.update();
    setTime(timeClient.getEpochTime());
    String timeString = String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second());
    Serial.println(timeString);
    saveFirebase("2", timeIn[1], timeString);
  } else {
    // The parking slot now is empty!!!
    Serial.println("Parking slot 2 is empty");
  }

  // 3
  // This mean a new car just entered
  if (lot_3 == LOW && isParked[2] == false) {
    Serial.println("A new car entered to slot 3");
    // save the time its entered and save to firebase
    // TIme
    timeClient.update();
    setTime(timeClient.getEpochTime());
    String timeString = String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second());
    Serial.println(timeString);
    isParked[2] = true;  // set the isParked to true
    timeIn[2] = timeString;
  } else if (lot_3 == LOW && isParked[2] == true) {
    // A new car is stay in the park, we already have save the time! Just print to the LCD that this slot is occupied
    Serial.println("Parking slot 3 is occupied!");
  } else if (lot_3 == HIGH && isParked[2] == true) {
    // A new car has just leave the parking slot!!! Save the leave time to firebase
    Serial.println("The car just leave the slot 3");
    isParked[2] = false;
    // TIme
    timeClient.update();
    setTime(timeClient.getEpochTime());
    String timeString = String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second());
    Serial.println(timeString);
    saveFirebase("3", timeIn[2], timeString);
  } else {
    // The parking slot now is empty!!!
    Serial.println("Parking slot 3 is empty");
  }
  // 4
  // This mean a new car just entered
  if (lot_4 == LOW && isParked[3] == false) {
    Serial.println("A new car entered to slot 4");
    // save the time its entered and save to firebase
    // TIme
    timeClient.update();
    setTime(timeClient.getEpochTime());
    String timeString = String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second());
    Serial.println(timeString);
    isParked[3] = true;  // set the isParked to true
    timeIn[3] = timeString;
  } else if (lot_4 == LOW && isParked[3] == true) {
    // A new car is stay in the park, we already have save the time! Just print to the LCD that this slot is occupied
    Serial.println("Parking slot 4 is occupied!");
  } else if (lot_4 == HIGH && isParked[3] == true) {
    // A new car has just leave the parking slot!!! Save the leave time to firebase
    Serial.println("The car just leave the slot 4");
    isParked[3] = false;
    // TIme
    timeClient.update();
    setTime(timeClient.getEpochTime());
    String timeString = String(day()) + "/" + String(month()) + "/" + String(year()) + " " + String(hour()) + ":" + String(minute()) + ":" + String(second());
    Serial.println(timeString);
    saveFirebase("4", timeIn[3], timeString);
  } else {
    // The parking slot now is empty!!!
    Serial.println("Parking slot 4 is empty");
  }
  Serial.println("---------------");
  delay(1000);
}

// thread function to control the gate
void controlGate() {
  int gate_in, gate_out;
  // check the car in or out to open the gate
  gate_in = digitalRead(irSensorGate0Pin);
  gate_out = digitalRead(irSensorGate1Pin);

  if (gate_in == LOW) {
    openGate();
    while (gate_in == LOW) {
      gate_in = digitalRead(irSensorGate0Pin);
    }
    closeGate();
  } else if (gate_out == LOW) {
    while (gate_out == LOW) {
      openGate();
      gate_out = digitalRead(irSensorGate1Pin);
    }
    closeGate();
  }
}

void openGate() {
  servo.write(180);
  delay(1000);
}

void closeGate() {
  delay(1000);
  servo.write(0);
}

void saveFirebase(String slot, String timeIn, String timeOut) {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    // get current timestamp
    timeClient.update();
    int timeStamp = timeClient.getEpochTime();
    String parentPath = "/" + String(timeStamp);
    json.set("/slot", slot);
    json.set("/time_in", timeIn);
    json.set("/time_out", timeOut);
    Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json);
  }
}


void setup() {
  // set the pins for ir sensors as input
  pinMode(irSensor1Pin, INPUT);
  pinMode(irSensor2Pin, INPUT);
  pinMode(irSensor3Pin, INPUT);
  pinMode(irSensor4Pin, INPUT);
  // set the pins for the gate
  pinMode(irSensorGate0Pin, INPUT);
  pinMode(irSensorGate1Pin, INPUT);

  // initialize the serial communication for debugging
  Serial.begin(9600);

  // //Initialize the Blynk library
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  Blynk.virtualWrite(V0, LOW);

  // initialize the servo object
  servo.attach(servoPin);

  // detach the threads to let them run independently
  detectParking_thread.onRun(detectParkingSlot);
  detectParking_thread.setInterval(250);
  controlGate_thread.onRun(controlGate);
  controlGate_thread.setInterval(250);

  controller.add(&controlGate_thread);
  controller.add(&detectParking_thread);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  timeClient.begin();
  // Firebase
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &fauth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &fauth);
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
}

void loop() {
  //Run the Blynk library
  Blynk.run();

  controller.run();
}