// Logical GPIO

#ifndef	l
#define	l(n,g)
#endif

l(GPSFault, 48)                 // GPS fault (no serial)
    l(GPSWarn, 49)              // GPS not locked
    l(KeyTamper, 56)            // Keypad tamper input
    l(KeyFault, 57)             // Keypad fault deteched
    l(NFCFault, 58)             // NFC fault detected
    l(LockFault, 59)            // Lock fault detected
    l(DoorForce, 60)            // Open by force or lock force
    l(DoorProp, 61)             // Unauthorised propped (i.e. NOTCLOSED state)
    l(MeshFault, 63)            // Lonely on mesh
// Note max 63 as byte as INV and SET
// Note we do not use inverted for these, so could extend using -48 to -63 if needed
#undef l
