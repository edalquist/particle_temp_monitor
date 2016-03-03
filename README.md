# particle_temp_monitor
Monitor temps via a Particle Photon and ThingSpeak using
[TMP36](https://www.sparkfun.com/products/109880) sensors.

# Dependencies
Clone [SparkCore Thingspeak](https://github.com/dareid/sparkcore.thingspeak) into a sibling
directory to satisfy the ```thingspeak.h``` and ```thingspeak.cpp``` links.

    git clone https://github.com/dareid/sparkcore.thingspeak.git

# Configuration
* Copy ```SAMPLE_credentials.h``` to ```credentials.h``` and enter your Thingspeak channel key.
* Update the ```SENSORS``` vector in ```particle_temp_monitor.ino``` to point to the correct pins
  for your TMP36 sensors. Also update the event labels and Thingspeak Field IDs
