#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>
#include <time.h>

// WiFi and Firebase configuration
#define WIFI_SSID "Network not Found"
#define WIFI_PASSWORD "thebestofthebest"
#define API_KEY "AIzaSyDuoOmODQyXouGNZ5HXvb5O3tdrb_lQVYA"
#define DATABASE_URL "https://smart-agrigreen-default-rtdb.asia-southeast1.firebasedatabase.app/"

// User credentials for Firebase Authentication
#define USER_EMAIL "mongarparjeet@gmail.com"  
#define USER_PASSWORD "parjeet2002"  

// Sensors and actuators setup
#define SOIL_SENSOR_PIN1 32
#define SOIL_SENSOR_PIN2 33
#define TEMPERATURE_SENSOR_PIN 21
#define RELAY_PIN 25
#define RELAY_PIN1 22
int soilThreshold = 2380;  // Default soil threshold
float temperatureThreshold = 20.0;  // Default temperature threshold

// Firebase and WiFi objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// DHT sensor setup
#define DHTTYPE DHT11
DHT dht(TEMPERATURE_SENSOR_PIN, DHTTYPE);

// Track connection state
unsigned long sendDataPrevMillis = 0;
bool loginOK = false;

// Time setup
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

String getTimeStamp() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "N/A";
  }
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &timeinfo);
  return String(buffer);
}

void tokenStatusCallback(TokenInfo info) {
  Serial.printf("Token Info: type = %d, status = ", info.type);
  switch (info.status) {
    case token_status_uninitialized:
      Serial.println("Uninitialized");
      break;
    case token_status_on_signing:
      Serial.println("On Signing");
      break;
    case token_status_on_request:
      Serial.println("On Request");
      break;
    case token_status_on_refresh:
      Serial.println("On Refresh");
      break;
    case token_status_ready:
      Serial.println("Ready");
      break;
    case token_status_error:
      Serial.println("Error");
      break;
    default:
      Serial.println("Unknown Status");
      break;
  }
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RELAY_PIN1, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(RELAY_PIN1, HIGH);
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// Fetch the latest threshold values from Firebase
void fetchThresholds() {
  // Fetch soil moisture threshold
  if (Firebase.getInt(fbdo, "/settings/thresholds/moisture/min")) {
    soilThreshold = fbdo.intData();
    Serial.print("Fetched soilThreshold: ");
    Serial.println(soilThreshold);
  } else {
    Serial.print("Failed to fetch soilThreshold: ");
    Serial.println(fbdo.errorReason());
  }

  // Fetch temperature threshold
  if (Firebase.getFloat(fbdo, "/settings/thresholds/temperature/max")) {
    temperatureThreshold = fbdo.floatData();
    Serial.print("Fetched temperatureThreshold: ");
    Serial.println(temperatureThreshold);
  } else {
    Serial.print("Failed to fetch temperatureThreshold: ");
    Serial.println(fbdo.errorReason());
  }
}

// Update pump frequency in Firebase
void incrementPumpFrequency() {
  // Get the current date
  String date;
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo); // Format: YYYY-MM-DD
    date = String(buffer);
  } else {
    Serial.println("Failed to get current date!");
    return;
  }
  // Firebase path for today's pump frequency
  String path = "/usage_statistics/daily_pump_frequency/" + date;

  // Get the current frequency for today
  if (Firebase.getInt(fbdo, path)) {
    int currentFrequency = fbdo.intData();
    Firebase.setInt(fbdo, path, currentFrequency + 1); // Increment frequency
    Serial.println("Pump frequency incremented for " + date);
  } else {
    // Initialize frequency for today if it doesn't exist
    Firebase.setInt(fbdo, path, 1);
    Serial.println("Pump frequency initialized for " + date);
  }
}


void loop() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 5000  sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    
    // Fetch updated threshold values from Firebase
    fetchThresholds();

    // Read soil moisture values
    int soilMoistureValue1 = analogRead(SOIL_SENSOR_PIN1);
    delay(1000);
    int soilMoistureValue2 = analogRead(SOIL_SENSOR_PIN2);
    delay(1000);

    // Read temperature
    float temperature = dht.readTemperature();  // Read temperature in Celsius

    Serial.println("Soil Moisture Value1: " + String(soilMoistureValue1));
    Serial.println("Soil Moisture Value2: " + String(soilMoistureValue2));
    Serial.println("Temperature: " + String(temperature) + " C");

    String timestamp = getTimeStamp();
    int pumpStatus = 0;
    int fanStatus = 0;

    Firebase.getInt(fbdo, "/manual_control/pump_status");
    int manualPumpValue = fbdo.intData();
    Firebase.getInt(fbdo, "/manual_control/fan_status");
    int manualFanValue = fbdo.intData();

    Serial.print("Manual Pump Status (from Firebase): ");
    Serial.println(manualPumpValue);
    Serial.print("Manual Fan Status (from Firebase): ");
    Serial.println(manualFanValue);

    bool manualPump = (manualPumpValue == 1);
    bool manualFan = (manualFanValue == 1);

    // Soil moisture control logic with manual override
    if (manualPump) {
      digitalWrite(RELAY_PIN, LOW);
      pumpStatus = 1;
      Serial.println("Manual Override: Pump is ON");
      delay(5000);

    } else if (soilMoistureValue1 < soilThreshold  soilMoistureValue2 < soilThreshold) {
      digitalWrite(RELAY_PIN, LOW);
      pumpStatus = 1;
      Serial.println("Automatic Control: Pump is ON");
      incrementPumpFrequency();  // Increment pump frequency when automatically activated
      delay(5000);
    } else {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Pump is OFF");
    }

    // Temperature control logic with manual override
    if (manualFan) {
      digitalWrite(RELAY_PIN1, LOW);
      fanStatus = 1;
      Serial.println("Manual Override: Fan is ON");
      delay(5000);
    } else if (temperature > temperatureThreshold) {
      digitalWrite(RELAY_PIN1, LOW);
      fanStatus = 1;
      Serial.println("Automatic Control: Fan is ON");
      delay(5000);
    } else {
      digitalWrite(RELAY_PIN1, HIGH);
      Serial.println("Fan is OFF");
    }

    // Store current sensor values
    if (Firebase.setFloat(fbdo, "/current/sensor/plot1/soil_moisture_value1", soilMoistureValue1) &&
        Firebase.setFloat(fbdo, "/current/sensor/plot1/soil_moisture_value2", soilMoistureValue2) &&
        Firebase.setFloat(fbdo, "/current/sensor/plot1/temperature", temperature)) {
      Serial.println("Current sensor values sent for plot1");
    } else {
      Serial.println("Error sending current sensor data: " + fbdo.errorReason());
    }

    // Store current actuator status
    if (Firebase.setInt(fbdo, "/current/actuator/pump_status", pumpStatus) &&
        Firebase.setInt(fbdo, "/current/actuator/fan_status", fanStatus)) {
      Serial.println("Current actuator status sent");
    } else {
      Serial.println("Error sending current actuator data: " + fbdo.errorReason());
    }
    // Store sensor history with timestamp
    if (Firebase.setFloat(fbdo, "/history/sensor/plot1/" + timestamp + "/soil_moisture_value1", soilMoistureValue1) &&
        Firebase.setFloat(fbdo, "/history/sensor/plot1/" + timestamp + "/soil_moisture_value2", soilMoistureValue2) &&
        Firebase.setFloat(fbdo, "/history/sensor/plot1/" + timestamp + "/temperature", temperature)) {
      Serial.println("Sensor history sent for plot1");
    } else {
      Serial.println("Error sending sensor history: " + fbdo.errorReason());
    }

    // Store actuator history with timestamp
    if (Firebase.setInt(fbdo, "/history/actuator/plot1/" + timestamp + "/pump_status", pumpStatus) &&
        Firebase.setInt(fbdo, "/history/actuator/plot1/" + timestamp + "/fan_status", fanStatus)) {
      Serial.println("Actuator history sent");
    } else {
      Serial.println("Error sending actuator history: " + fbdo.errorReason());
    }
    // Reset manual control to off
    Firebase.setInt(fbdo, "/manual_control/pump_status", 0);
    Firebase.setInt(fbdo, "/manual_control/fan_status", 0);
  }
}
