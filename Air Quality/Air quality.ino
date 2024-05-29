#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>



//------------------------------Dht11---------------------------------
#include <DHT11.h>

DHT11 dht11(25);

#define Ventilator  18 // ESP32 pin GPIO18 connected to relay

#define TEMP_UPPER_THRESHOLD  30 // upper temperature threshold




//-------------------------Gas sensor--------------------------------
#define gas_sensor_pin 34
#define buzzer 2


const char* ssid = "Deepthyka";
const char* password = "11122002";
const char* mqttServer = "broker.emqx.io"; // MQTT broker address
const int mqttPort = 1883; // MQTT broker port
const char* mqttUser = ""; // MQTT username (if required)
const char* mqttPassword = ""; // MQTT password (if required)

//DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

    Serial.begin(115200);
    WiFi.begin(ssid, password);

    //--------------Buzzerr--------------------
    pinMode(buzzer, OUTPUT);
    
    //--------------Ventilator--------------------
    pinMode(Ventilator, OUTPUT);

    


    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");
    client.setServer(mqttServer, mqttPort);
   
}

void reconnect() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");
        if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
            Serial.println("Connected to MQTT broker");
        }
        else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void loop() {


  //--------------------------------Dht11------------------------------
    int temperature = 0;
    int humidity = 0;

    int result = dht11.readTemperatureHumidity(temperature, humidity);
    // Check the results of the readings.
    // If the reading is successful, print the temperature and humidity values.
    // If there are errors, print the appropriate error messages.
    if (result == 0) {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" °C\tHumidity: ");
        Serial.print(humidity);
        Serial.println(" %");
    } else {
        // Print error message based on the error code.
        Serial.println(DHT11::getErrorString(result));
    }
    
    if (temperature > TEMP_UPPER_THRESHOLD) {
      Serial.println("The fan is turned on !!!   Temperature is high!!!");
      digitalWrite(Ventilator, HIGH); // turn on
    } else {
      Serial.println("The fan is turned off....    Temperature is normal...");
      digitalWrite(Ventilator, LOW); // turn off
    }
  


    

   
  //--------------------------------Gas Sensor-------------------------
  int gasvalue = analogRead(gas_sensor_pin);
  gasvalue = map(gasvalue, 0, 4095, 0, 100);
  Serial.print("Gas Value : ");
  Serial.print(gasvalue);
  Serial.println("%");

  if (gasvalue >= 50) {
    digitalWrite(buzzer, HIGH);
    Serial.println("Warning! Gas concentration is High!!! ");
    
  } else {
    digitalWrite(buzzer, LOW);
    Serial.println("Normal.... Gas concentration is normal....");
  }

  

  
    if (!client.connected()) {
        reconnect();
    }

    String data = String(temperature) + "," + String(humidity) + "," + String(gasvalue);
    
    String topic = "Air_Quality"; // MQTT topic to publish to
    
    client.publish(topic.c_str(), data.c_str());

    Serial.println("Published to MQTT: " + data);
    delay(1000);
}
