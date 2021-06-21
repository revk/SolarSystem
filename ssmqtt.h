// MQTT handling

typedef char device_t[12];
typedef char fob_t[14];
typedef char aid_t[6];

void mqtt_start(void);
void command(j_t*);// Send command j."command" to j."device", and free j
void setting(j_t*);// Send setting to j."device", and free j


