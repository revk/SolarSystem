// Logical inputs

#include "SS.h"
#include <driver/gpio.h>

// Input ports
#define MAXINPUT 26
#define	BITFIELDS "-"
#define	PORT_INV 0x40
#define	port_mask(p) ((p)&63)
static uint8_t input[MAXINPUT];

// Other settings
#define settings	\
u32 (inputhold, 100);	\
u32 (inputpoll, 10);	\

static TaskHandle_t input_task_id = NULL;
#define u32(n,v) uint32_t n
settings
#undef u32
static uint64_t input_raw = 0;
static uint64_t input_stable = 0;
static uint64_t port_invert = 0;
static uint64_t input_hold[MAXINPUT] = { };

static volatile char reportall = 0;

const char *
input_command (const char *tag, unsigned int len, const unsigned char *value)
{
   if (!strcmp (tag, "connect"))
      reportall = 1;
   return NULL;
}

static void
input_task (void *pvParameters)
{                               // Main RevK task
   pvParameters = pvParameters;
   // Scan inputs
   while (1)
   {
      // Check inputs
      int64_t now = esp_timer_get_time ();
      char report = reportall;
      reportall = 0;
      int i;
      for (i = 0; i < MAXINPUT; i++)
         if (input[i])
         {
            int v = gpio_get_level (port_mask (input[i]));
            if ((1ULL << i) & port_invert)
               v = 1 - v;
	    char changed=report;
            if ((input_hold[i] < now) && (report || v != ((input_stable >> i) & 1)))
            {                   // Change of stable state
               input_stable = ((input_stable & ~(1ULL << i)) | ((uint64_t) v << i));
	       changed=1;
            }
	    if(changed)
	    {
               char tag[20];
               sprintf (tag, "input%d", i + 1);
               revk_state (tag, "%d", (input_stable>>i)&1);
	    }
            if (v != ((input_raw >> i) & 1))
            {                   // Change of raw state
               input_raw = ((input_raw & ~(1ULL << i)) | ((uint64_t) v << i));
               input_hold[i] = now + inputhold * 1000;
            }
         }
      // Sleep
      usleep ((inputpoll ? : 1) * 1000);
   }
}

void
input_init (void)
{
   revk_register ("input", MAXINPUT, sizeof (*input), &input, BITFIELDS, SETTING_BITFIELD | SETTING_SET);
#define u32(n,v) revk_register(#n,0,sizeof(n),&n,#v,0);
   settings
#undef u32
      // Check config
   int i,
     p;
   for (i = 0; i < MAXINPUT; i++)
      if (input[i] && port_ok (p = port_mask (input[i]), "input"))
      {
         REVK_ERR_CHECK (gpio_reset_pin (p));
         REVK_ERR_CHECK (gpio_set_pull_mode (p, GPIO_PULLUP_ONLY));
         REVK_ERR_CHECK (gpio_set_direction (p, GPIO_MODE_INPUT));
         if (input[i] & PORT_INV)
            port_invert |= (1ULL << i); // TODO can this not be done at hardware level?
      } else
         input[i] = 0;
   xTaskCreatePinnedToCore (input_task, "input", 16 * 1024, NULL, 1, &input_task_id, tskNO_AFFINITY);   // TODO stack, priority, affinity check?
}
