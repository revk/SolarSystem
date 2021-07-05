// MQTT handling

typedef char device_t[12 + 1];  // Hex device ID
typedef char fob_t[14 + 1];     // Hex fob ID
typedef char aid_t[6 + 1];      // Hex AID

void mqtt_start(void);

void mqtt_qin(j_t *);           // Queue incoming JSON
j_t incoming(void);             // Wait for and get next incoming message (not just used by MQTT, includes _meta)

// Server slot functions.
typedef long long slot_t;       // Non zero no reusing
slot_t slot_create(int *, const char *);        // Create a slot, and allocated the non blocking linked sockets, returns server socket
slot_t slot_linked(slot_t);     // Return the linked slot (cleared if linked slot has been destroyed)
void slot_link(slot_t, slot_t); // Link two slots, both show slot_linked as the other
void slot_close(slot_t);        // Sends the server a zero length message to the server asking it to close and destroy
void slot_destroy(slot_t);      // Destroy a slot, called by server when closing, handles socket closing, unlinks
const char *slot_send(slot_t, const char *prefix, const char *suffix, j_t *);   // Set MQTT formatted message to a slot
void slot_setdeviceid(slot_t id, device_t deviceid);
