

#include "contiki.h"
#include "httpd-simple.h"
#include "dev/sht11-sensor.h"
#include "dev/light-sensor.h"
#include "dev/leds.h"
#include <stdio.h>

PROCESS(web_sense_process, "Sense Web Demo");
PROCESS(webserver_nogui_process, "Web server");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
AUTOSTART_PROCESSES(&web_sense_process,&webserver_nogui_process);

#define HISTORY 16
static int temperature[HISTORY];
static int light1[HISTORY];

/*---------------------------------------------------------------------------*/
static int
get_light(void)
{
  return 10 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC) / 7;
}

static int
get_hum(void)
{
 /*- return (((0.0405*sht11_sensor.value(SHT11_SENSOR_HUMIDITY)) - 4) + ((-2.8 * 0.000001)*(sht11_sensor.value(SHT11_SENSOR_HUMIDITY)))); --*/
static int dec;  
dec = (((0.0405*sht11_sensor.value(SHT11_SENSOR_HUMIDITY)) - 4) + ((-2.8 * 0.000001)*(sht11_sensor.value(SHT11_SENSOR_HUMIDITY))));
return dec;
}
/*---------------------------------------------------------------------------*/
static int
get_temp(void)
{
  return ((sht11_sensor.value(SHT11_SENSOR_TEMP) / 10) - 396) / 10;
}
/*---------------------------------------------------------------------------*/
static const char *TOP = "<html><head><title>Sens</title></head><body>\n";
static const char *BOTTOM = "</body></html>\n";
/*---------------------------------------------------------------------------*/
/* Only one single request at time */
static char buf[256];
static int blen;
#define ADD(...) do {                                                   \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
  } while(0)

static
PT_THREAD(send_values(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

  SEND_STRING(&s->sout, TOP);

  if(strncmp(s->filename, "/index", 6) == 0 ||
     s->filename[1] == '\0') {

    blen = 0;
    ADD("L: %u "",T: %u&deg;C,""H: %u",
        get_light(), get_temp(),get_hum());
    SEND_STRING(&s->sout, buf);

  } 
  

  SEND_STRING(&s->sout, BOTTOM);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return send_values;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(web_sense_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();

  etimer_set(&timer, CLOCK_SECOND * 2);
  SENSORS_ACTIVATE(light_sensor);
  SENSORS_ACTIVATE(sht11_sensor);

 

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/