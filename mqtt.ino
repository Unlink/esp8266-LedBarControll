long mqttLastReconnectAttempt = 0;

boolean mqttReconnect() {
  mqttClient.connect("esp-osvetlenie");
  return mqttClient.connected();
}
