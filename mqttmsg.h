
void mqtt_topic(j_t j, const char *topic, int tlen);    // Break down _meta.topic in to prefix and suffix (optionally store it too)
void mqtt_dataonly(j_t j);      // Remove meta
j_t mqtt_decode(unsigned char *, size_t);       // Decode MQTT publish
size_t mqtt_encode(unsigned char *, size_t, const char *topic, j_t);    // Encode MQTT publish
