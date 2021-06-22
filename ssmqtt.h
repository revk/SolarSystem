// MQTT handling

typedef char device_t[12+1];	// Hex device ID
typedef char fob_t[14+1];	// Hex fob ID
typedef char aid_t[6+1];	// Hex AID

void mqtt_start(void);
void command(j_t *);            // Send command j."command" to j."device", and free j
void setting(j_t *);            // Send setting to j."device", and free j
j_t incoming(void);		// Wait for and get next incoming message
