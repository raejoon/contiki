#define WITH_TINYOS_AUTO_IDS 1

#define SOLO_CONF_ENABLE 1
#define SOLO_CONF_START_AT_BOOT 0
#define SOLO_CONF_PCO_ENABLE 1
#define SOLO_CONF_CLAMPING_ENABLE 1
#define SOLO_CONF_LOOP_DETECT_ENABLE 1
#define SOLO_CONF_INTERVAL (8 * CLOCK_SECOND)
#define SOLO_CONF_INTERVAL_THRESHOLD (4*SOLO_CONF_INTERVAL)
#define SOLO_CONF_TIMESTAMP_THRESHOLD (8*SOLO_CONF_INTERVAL)

#define CC2420_CONF_SFD_TIMESTAMPS 1