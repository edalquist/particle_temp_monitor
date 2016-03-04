# particle_temp_monitor
Monitor temps via a Particle Photon and ThingSpeak using
[TMP36](https://www.sparkfun.com/products/109880) sensors.

# Dependencies
Clone [SparkCore Thingspeak](https://github.com/dareid/sparkcore.thingspeak) and
[HttpClient](https://github.com/nmattisson/HttpClient) into a sibling directory to satisfy the
symbolic links for the libraries.

    git clone https://github.com/dareid/sparkcore.thingspeak.git
    git clone https://github.com/nmattisson/HttpClient.git

# Configuration
* Copy ```SAMPLE_credentials.h``` to ```credentials.h```
* Enter your Thingspeak channel key.
* Enter your Ubidots key and variable IDs
* Update the ```SENSORS``` vector in ```particle_temp_monitor.ino``` to point to the correct pins
  for your TMP36 sensors. Also update the event labels and Thingspeak Field IDs
