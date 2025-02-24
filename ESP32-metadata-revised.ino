#include "BluetoothA2DPSink.h"

BluetoothA2DPSink a2dp_sink;

void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
  Serial.printf("Metadata: id: %d, text: %s\n", id, text);
}

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr) {
  if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
    Serial.println("Connected");
  } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
    Serial.println("Disconnected");
  }
}

void setup() {
  Serial.begin(115200); // Initialize serial communication for debugging

  // Set the metadata callback
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);

  // Set the connection state callback
  a2dp_sink.set_on_connection_state_changed(connection_state_changed);

  // Start the A2DP sink with a device name
  a2dp_sink.start("ESP32_A2DP_Receiver");
}

void loop() {
  // No need to do anything here
}
