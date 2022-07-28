// Logical GPIO

#ifndef	l
#define	l(n,g)
#endif

// 48-55 are for NFC
// 56-63 are for logical and -56 to -63
// Can also use negative values -48 to -64

    l(KeyTamper, 56)            // Keypad tamper input
    l(KeyFault, 57)             // Keypad fault deteched
    l(NFCFault, 58)             // NFC fault detected
    l(LockFault, 59)            // Lock fault detected
    l(DoorForce, 60)            // Open by force or lock force
    l(DoorProp, 61)             // Unauthorised propped (i.e. NOTCLOSED state)
    l(GPSFault, 62)             // GPS fault (no serial)
    l(MeshFault, 63)            // Lonely on mesh

    l(Timer1,-56)		// Internal timer event
    l(GPSNoSats, -62)           // GPS No sats

// Note max 63 as byte as INV and SET
#undef l
