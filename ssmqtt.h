// MQTT handling

typedef char device_t[12 + 1];  // Hex device ID
typedef char fob_t[14 + 1];     // Hex fob ID
typedef char aid_t[6 + 1];      // Hex AID

void mqtt_start(void);
const char *mqtt_send(long long instance, const char *prefix, const char *suffix, j_t *);       // Set MQTT
const char *command(long long instance, const char *suffix, j_t *);     // Send command 
const char *setting(long long instance, const char *suffix, j_t *);     // Send setting 

typedef struct slot_s slot_t;
void mqtt_qin(j_t *);           // Queue incoming
slot_t *mqtt_slot(int *txsockp);        // Create a slot
void mqtt_close_slot(long long instance);
void slot_link(long long instance, slot_t *target);
long long slot_linked(long long instance);

j_t mqtt_decode(unsigned char *buf,size_t len); // Decode and MQTT message, return JSON payload, with topic in _meta.topic

j_t incoming(void);             // Wait for and get next incoming message (not just used by MQTT, includes _meta)
