/*
 * (C) Copyright 2016 Eric Dalquist.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "thingspeak.h"
#include "HttpClient.h"
#include "credentials.h"
#include <vector>
#include <map>

ThingSpeakLibrary::ThingSpeak thingspeak (THINGSPEAK_KEY);
HttpClient http;
// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
   { "Content-Type", "application/json" },
   { "X-Auth-Token", UBIDOTS_KEY},
   { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;


// Samples to average
const int WINDOW_SIZE = 300;
// Time in seconds between updates
const int UPDATE_INTERVAL = 15;
// Time in seconds beetween samples
const int SAMPLE_INTERVAL = 1;

const double UNSET_TEMP = std::numeric_limits<double>::max();

// Sensor config
struct SensorDesc {
    String desc;
    int pin;
    int thingspeakFieldId;
    String ubidotsVar;
};

// Define our 3 sensors
std::vector<SensorDesc> SENSORS = {
    {"Outside", A1, 2, UBIDOTS_VAR_OUTSIDE},
    {"Top",     A0, 1, UBIDOTS_VAR_FREEZER_TOP},
    {"Bottom",  A2, 3, UBIDOTS_VAR_FREEZER_BOTTOM}
};

// list of list of doubles that will track sensor readings across all the sensors
std::vector<std::vector<double>> TEMPS(SENSORS.size(), std::vector<double>(WINDOW_SIZE, UNSET_TEMP));

// Last position written to in the vector
int tempIdx = 0;

// Last time an update was sent
int lastUpdate = 0;


void setup() {
    Serial.begin(9600);

    // Init the sensor pins
    for (const auto &sensor : SENSORS) {
        String sensorDesc = String::format("pin %d for %s", sensor.pin, sensor.desc.c_str());
        Serial.println(sensorDesc);
        Particle.publish("set_input", sensorDesc);
        pinMode(sensor.pin, INPUT);
    }

    request.hostname = "things.ubidots.com";
    request.port = 80;
    request.path = "/api/v1.6/collections/values";

    // Set lastUpdate to at least one interval into the future
    lastUpdate = Time.now() + UPDATE_INTERVAL;
}

double readTempF(int pin) {
    Serial.println(String::format("Read input pin %d", pin));
    int analogVal = analogRead(pin);

    // // Read the temp sensor value as mV. 12 bit range over 3.3V input
    // double tempV = (analogVal / (double) 4095) * 3300;

    // // TMP36 outputs 750mV at 25C and scales at 10mV/C
    // // Math is using to 500mV at 0C to simplify the calculations
    // double tempC = (tempV - 500) / (double) 10;

    // // Convert C to F
    // return ((tempC * 9) / 5) + 32;

    /*
     * Wolfram Alpha Simplification of the above math. Reduces 7 floating point operations to two
     * http://www.wolframalpha.com/input/?i=((((((X+%2F+4095)+*+3300)+-+500)+%2F+10)+*+9)+%2F+5)+%2B+32
     */
    return ((66 * analogVal) / (double) 455) - 58;
}

void loop() {
    Serial.println("Loop start");

    // Capture
    int sensorIdx = 0;
    for (const auto &sensor : SENSORS) {
        TEMPS[sensorIdx][tempIdx] = readTempF(sensor.pin);
        sensorIdx++;
    }
    tempIdx = (tempIdx + 1) % WINDOW_SIZE;

    if (lastUpdate + UPDATE_INTERVAL <= Time.now()) {
        String eventData = "";
        String ubidotsData = "[";

        // Iterate over all of the sensors
        int sensorIdx = 0;
        for (const auto &sensor : SENSORS) {
            std::vector<double> sensorTemps = TEMPS[sensorIdx];

            // Average the values in the sensorTemps vector
            double avg = 0;
            int tempIdx = 0;
            for (const auto &temp : TEMPS[sensorIdx]) {
                if (temp == UNSET_TEMP) {
                    break;
                }
                avg += (temp - avg) / ++tempIdx;
            }

            // Record average in thingspeak API and build on the event string
            String avgStr = String::format("%.2f", avg);

            thingspeak.recordValue(sensor.thingspeakFieldId, avgStr);

            eventData.concat(sensor.desc);
            eventData.concat(": ");
            eventData.concat(avgStr);

            ubidotsData.concat(String::format("{\"variable\": \"%s\", \"value\":%s}", sensor.ubidotsVar.c_str(), avgStr.c_str()));

            // Increment sensor index and append comma to event string if this isn't the last sensor
            if (++sensorIdx < SENSORS.size()) {
                eventData.concat(", ");
                ubidotsData.concat(", ");
            }
        }

        ubidotsData.concat("]");

        // Publish data
        thingspeak.sendValues();
        Particle.publish("temps", eventData);

        // Post data to ubidots
        request.body = ubidotsData;
        http.post(request, response, headers);
        
        lastUpdate = Time.now();
    }

    delay(SAMPLE_INTERVAL * 1000);
}
