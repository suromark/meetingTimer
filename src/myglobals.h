uint8_t brite = 2; // display brightness level 0-7

volatile unsigned long thisTick = 0; // gets set to millis() at the beginning of each loop()
volatile unsigned long totalSeconds = 0; // Default start position 

#define BUTTON D3
#define INTERNAL_LED D4
#define panelCS D8

#define BUTTON_TIME_LONG 1000   // seconds for holding for menu
#define BUTTON_TIME_LONGER 9999   // not used, not reachable if used with touch module due to its internal auto-reset ...

union IntToTwo { 
int16_t INT;
uint8_t bytes[2];
};

IntToTwo scrollDelay;

unsigned long stepDisplay = 60; // millis between display updates
unsigned long holdDisplay = 0;   // optional absolute hold delay in ms to suppress regular countdown updates (is reduced by stepDisplay every loop)

#define MODE_RUN 1
#define MODE_STOP 2
#define MODE_CONFIG 3
#define MODE_UP 4
#define MODE_DOWN 5
#define MODE_ALERT 6
uint8_t systemMode = MODE_STOP;
uint8_t systemDirection = MODE_UP;
uint8_t digitCursor = 0; // a cursor used during config mode; starts with 0 = leftmost hour digit, goes to 3 = rightmost minute digit (HH:MM)

unsigned long stepKeys = 10; // how often the input is polled for debouncing

// Dot Matrix Display Strip

int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
byte panelRotation = 1; // 0-3

// Texts

char t_fserr[] = "FSERR";
char t_boot[] = "Boot";
char t_config[] = "Config";
char t_up[] = "UP";
char t_ready[] = "Ready";
char t_down[] = "DOWN";
char t_stop[] = "STOP";
char t_alert[] = "ALARM";