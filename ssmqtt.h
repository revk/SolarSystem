// MQTT handling

typedef char device_t[12 + 1];  // Hex device ID
typedef char fob_t[14 + 1];     // Hex fob ID
typedef char aid_t[6 + 1];      // Hex AID

void mqtt_start(void);
const char *mqtt_send(long long instance, const char *prefix, const char *suffix, j_t *);       // Set MQTT
const char *command(long long instance, const char *suffix, j_t *);     // Send command 
const char *setting(long long instance, const char *suffix, j_t *);     // Send setting 

j_t incoming(void);             // Wait for and get next incoming message (not just used by MQTT, includes _meta)
