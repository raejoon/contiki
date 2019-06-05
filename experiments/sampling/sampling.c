#include "contiki.h"
#include "dev/sht11/sht11-sensor.h"
#include <stdio.h>

#define SAMPLING_INTERVAL ( 10 * CLOCK_SECOND )

static uint16_t temperature;

PROCESS(sampling_process, "Test sampling process");
AUTOSTART_PROCESSES(&sampling_process);

PROCESS_THREAD(sampling_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();
  SENSORS_ACTIVATE(sht11_sensor);

  while (1) {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    temperature = sht11_sensor.value(SHT11_SENSOR_TEMP);
    printf("Temperature: %u\n", temperature);
  }

  PROCESS_END();
}
