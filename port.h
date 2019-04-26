// Port structures

#define	MAX_BUS	10		// Maximum buses
#define	MAX_DEVICE (MAX_BUS*256)	// 256 addresses per bus

#define port_device(w) ((w)?(((w)->busid)>>8):0)
#define port_bits(w) ((w)?(((w)->busid)&0xFF):0)

typedef struct port_s port_t;
typedef port_t *port_p;
struct port_s
{
	port_p next;
	unsigned int busid;	// Bus ID (for RS485 bus logic), 0 for not on RS485

};

extern port_p ports;

port_p port_new_bus(unsigned int);
