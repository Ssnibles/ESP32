#include "BluetoothA2DPSink.h"

BluetoothA2DPSink a2dp_sink;

void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
  Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
}

void setup() {
    Serial.begin(115200);
    
    // Set up the metadata callback
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    
    // Start the A2DP sink
    a2dp_sink.start("ESP32");
}

void loop() {
    // The loop can remain empty as the callbacks handle the events
}