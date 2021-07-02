// Port structures

#ifndef PORT_H
#define PORT_H

#define	MAX_BUS	10		// Maximum buses
#define	MAX_DEVICE (MAX_BUS*256)	// 256 addresses per bus

#define port_device(w) ((w)?((((unsigned int)(w)->bus)<<8)+(w)->id):0)
#define port_bus(w) ((w)?(w)->bus:0)
#define port_id(w) ((w)?(w)->id:0)
#define port_port(w) ((w)?(w)->port:0)
#define port_bits(w) ((w)&&(w)->port?(1<<((w)->port-1)):0)
#define port_isinput(w) ((w)&&(w)->port&&(w)->isinput)
#define port_isoutput(w) ((w)&&(w)->port&&!(w)->isinput)
#define port_mqtt(w) ((w)?(w)->mqtt:NULL)

typedef struct port_app_s port_app_t;
typedef struct port_s port_t;
typedef port_t *port_p;
struct port_s
{
  port_p next;
  // About the port
  const char *name;
  // MQTT based device
  const char *mqtt;		// MQTT Device ID (NULL for bus based)
  // Bus based port/device
  unsigned char bus;		// Bus ID (0 for MQTT)
  unsigned char id;		// Bus ID (0 for MQTT)
  // Port info
  unsigned char port;		// Port number - from 1. 0 means not a port (i.e. device level record for things like tamper/fault)
  unsigned char isinput:1;	// This is an input port
  // Actual state
  unsigned char state:1;	// Current actual state (updated on post event)
  unsigned char fault:1;	//
  unsigned char tamper:1;	//
  // Application data
  port_app_t *app;
};

extern port_p ports;

void port_start (void);
port_p port_new_bus (unsigned char bus,	// Bus ID (from 0)
		     unsigned char id,	// Device ID on bus
		     unsigned char isinput,	// 1 if input
		     unsigned char port);	// port number, from 1 (0 means device level)
port_p port_new (const char *mqtt,	// Device id (12 char hex normally)
		 unsigned char isinput,	// 1 if input
		 unsigned char port);	// port number, from 1 (0 means device level)
port_p port_new_base (port_p parent, unsigned char isinput, unsigned char port);	// Based on parent port

#define port_defined(w) port_defined_n((volatile port_p*)&(w),sizeof(w)/sizeof(port_p))
int port_defined_n (volatile port_p * w, int n);

#define port_input(w) port_input_n((volatile port_p*)&(w),sizeof(w)/sizeof(port_p))
int port_input_n (volatile port_p * w, int n);

#define port_found(w) port_found_n((volatile port_p*)&(w),sizeof(w)/sizeof(port_p))
int port_found_n (volatile port_p * w, int n);

#define port_tamper(w) port_tamper_n((volatile port_p*)&(w),sizeof(w)/sizeof(port_p))
int port_tamper_n (volatile port_p * w, int n);

#define port_output(w,v) port_output_n((volatile port_p*)&(w),sizeof(w)/sizeof(port_p),v)
void port_output_n (volatile port_p * w, int n, int v);

#define port_urgent(w) port_urgent_n((volatile port_p*)&(w),sizeof(w)/sizeof(port_p))
void port_urgent_n (volatile port_p * w, int n);

#endif
