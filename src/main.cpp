/*
Wiring: Wemos D1 mini -> Max7219 LED 8x8 matrix display chain
    5 V -> Vcc
    GND -> GND
    D8 HSPI-CS -> CS
    D6 HSPI-MISO -> -- not used --
    D7 HSPI-MOSI -> Din
    D5 HSPI-CLK -> CLK

Wiring: Action Button
    D3 <- Switch <- GND


I prefer to add 500-ish Ohms resistors along all ESP I/O lines, 
just in case I mess up the coding 
and/or I accidentally plug in a microcontroller with pins already set to output
and/or the signal isn’t at 3.3 volts level, to limit the current.
*/

#include <Arduino.h>
#include <FS.h>
#include "myglobals.h"

void doInput();
bool my_spiffs_setup();
void doStartStop();
void doDisplay();
void doDigitCycle();
void doCounting();
void doAlertEffect1();

/*
Own stuff
*/
#include "MySignal.h"
MySignal led;

#include "MyMatrix.h"
MyMatrix myma = MyMatrix(panelCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

#include "MyButtons.h"
MyButtons mbut = MyButtons(BUTTON, BUTTON_TIME_LONG, BUTTON_TIME_LONGER, stepKeys);

// Wifistuff

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
WiFiClient espClient;

/*




*/
void setup()
{
    Serial.begin(74880);

    pinMode(BUTTON, INPUT_PULLUP);

    led.activate(INTERNAL_LED, LOW, HIGH);

    myma.SetLevel(1);            // Use a value between 0 and 15 for brightness, but 0-7 usually suffices and uses less power
    myma.runInit(panelRotation); // set rotation of panels (adjust file data/orient.txt to your modules' layout), and do a bit of boot deco

    Serial.println(ESP.getChipId(), 16);

    Serial.println("Wifi OFF");
    WiFi.persistent(false); // avoid FLASH wear-out?

    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);

    if (my_spiffs_setup() == false)
    {
        myma.SetTextBuffer(t_fserr);
        myma.ShowCompactCentered(100);
        delay(10000);
        ESP.restart();
        delay(10000);
    }
    else
    {
        myma.setRotationFromFile();
        myma.SetTextBuffer(t_boot);
        myma.ShowCompactCentered(100);
        delay(500);
    }

    systemMode = MODE_STOP;
    myma.disableFlashColon();
}

/**
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * Hauptschleife
 * */
void loop()
{

    thisTick = millis();

    led.check();

    doInput();

    doCounting();

    doDisplay();
}

/*



   Spiffs start -- no formatting, should be taken care of during HTML pages upload (which is required for operation)
*/
bool my_spiffs_setup()
{

    Serial.println("SPIFFS is being mounted.");

    if (!SPIFFS.begin())
    {
        Serial.println("SPIFFS mount failure.");
        Serial.println("System will reboot in 10 seconds.");
        delay(10000);
        ESP.restart();
        delay(10000);
        return false;
    }
    pinMode(INTERNAL_LED, OUTPUT);
    return true;
}

void doInput()
{
    static unsigned long nextKeys = 0; // the next millis() value to do button polling
    static char modeinfo[16];

    if (thisTick > nextKeys)
    {
        nextKeys = thisTick + stepKeys;
        mbut.check();
        if (mbut.hold_long())
        {
            led.on();
        }

        if (mbut.release_short()) // cycle through display modes
        {
            if (systemMode == MODE_CONFIG)
            {
                doDigitCycle();
            }
            else
            {
                doStartStop();
            }
        }

        if (mbut.hold_long())
        {
            if (systemMode != MODE_CONFIG)
            {
                myma.SetTextBuffer(t_config);
                myma.ShowCompactCentered(0);
                holdDisplay = 700;
            }
            else
            {
                myma.ShowCompactCentered(0, digitCursor + 1 + (digitCursor > 0 ? 1 : 0));
            }
        }
        if (mbut.release_long())
        {
            myma.disableFlashColon();
            if (systemMode != MODE_CONFIG)
            {
                systemMode = MODE_CONFIG;
                digitCursor = 0;
                totalSeconds = 0;
                holdDisplay = 700;
                myma.SetTextBuffer(t_config);
                myma.ShowCompactCentered(0);
                Serial.println("Enter Config");
            }
            else
            {
                digitCursor = digitCursor + 1;
                if (digitCursor == 4)
                {
                    systemMode = MODE_STOP;
                    systemDirection = MODE_DOWN;
                    holdDisplay = 700;
                    myma.SetTextBuffer(t_ready);
                    myma.ShowCompactCentered(0);
                    Serial.println("Leave Config");
                }
                else
                {
                    /* no longer required after flashing cursor implementation */
                    /* holdDisplay = 700;
                    snprintf(modeinfo, sizeof(modeinfo), "Pos.%d", digitCursor + 1);
                    myma.SetTextBuffer(modeinfo);
                    myma.ShowCompactCentered(0);
                    */
                }
            }
            mbut.check();
        }
    }
}

void doStartStop()
{

    if (systemMode == MODE_ALERT)
    {
        systemMode = MODE_STOP;
        return;
    }

    if (systemMode == MODE_RUN)
    {
        systemMode = MODE_STOP;
        myma.disableFlashColon();
        holdDisplay = 700;
        myma.SetTextBuffer(t_stop);
        myma.ShowCompactCentered(0);
        Serial.println("Stop");
    }
    else
    {
        systemMode = MODE_RUN;
        holdDisplay = 700;
        if (systemDirection == MODE_UP)
        {
            myma.SetTextBuffer(t_up);
        }
        else
        {
            myma.SetTextBuffer(t_down);
        }
        myma.ShowCompactCentered(0);
        myma.enableFlashColon();
        Serial.println("Running");
    }
}

void doDigitCycle()
{
    unsigned long h10 = 0;
    unsigned long h1 = 0;
    unsigned long m10 = 0;
    unsigned long m1 = 0;

    // split the total seconds into digits

    h10 = totalSeconds / (10 * 60 * 60);

    h1 = (totalSeconds / (60 * 60)) % 10;

    m10 = (totalSeconds / 600) % 6;

    m1 = ((totalSeconds / 60) % 10);

    // process the digits
    if (digitCursor == 0)
    {
        h10 = (h10 + 1) % 10; // Cycle 10-hours up to 9
    }
    if (digitCursor == 1)
    {
        h1 = (h1 + 1) % 10; // Cycle 1-hours up to 9
    }
    if (digitCursor == 2)
    {
        m10 = (m10 + 1) % 6; // Cycle 10-minutes up to 5
    }
    if (digitCursor == 3)
    {
        m1 = (m1 + 1) % 10; // Cycle 1-minute up to 9
    }
    // recombine the time into seconds

    totalSeconds = (h10 * 10 * 60 * 60) + (h1 * 60 * 60) + (m10 * 10 * 60) + (m1 * 60);
}

/**
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */
void doDisplay()
{
    char clockview[32];

    static unsigned long lastTick = 0;

    static int16_t walker = 0;

    if ((thisTick - lastTick) < stepDisplay)
    {
        return;
    }

    lastTick = thisTick;

    if (holdDisplay > stepDisplay)
    {
        holdDisplay = holdDisplay - stepDisplay;
        return;
    }
    else
    {
        holdDisplay = 0;
    }

    if (systemMode == MODE_ALERT)
    {
        if (true)
        { // scaling circle effect
            doAlertEffect1();
            return;
        }

        // alternate flash effect

        if (thisTick % 1000 > 500)
        {
            myma.Fill(0);
            myma.matrix->fillRect(0, 0, myma.matrix->width() / 2, myma.matrix->height(), 1);
        }
        else
        {
            myma.Fill(1);
            myma.matrix->fillRect(0, 0, myma.matrix->width() / 2, myma.matrix->height(), 0);
        }
        myma.display();
        return;
    }

    if (totalSeconds >= 3600 || systemMode == MODE_CONFIG) // Stunde:Minute oder Minute:Sekunde?
    {
        snprintf(clockview, sizeof(clockview), "%02ld:%02ld", (totalSeconds / (60 * 60)), ((totalSeconds / 60) % 60));
    }
    else
    {
        snprintf(clockview, sizeof(clockview), "%02ld:%02ld", (totalSeconds / 60), (totalSeconds % 60));
    }

    myma.SetTextBuffer(clockview);
    if (systemMode == MODE_CONFIG)
    {
        myma.ShowCompactCentered(0, digitCursor + (digitCursor > 1 ? 1 : 0)); // flash the digit the cursor points to
    }
    else
    {
        myma.ShowCompactCentered(0);
    }

    // moving dots edges
    if (systemMode == MODE_RUN)
    {
        myma.matrix->drawPixel(0, walker, 1);
        myma.matrix->drawPixel(myma.matrix->width() - 1, walker, 1);
        if (systemDirection == MODE_DOWN)
        {
            walker = ((walker + 1) % myma.matrix->height());
        }
        else
        {
            walker = (walker > 0 ? (walker - 1) : myma.matrix->height() - 1);
        }
    }

    myma.display();
}

void doCounting()
{
    static unsigned long lastUpdate = 0;

    if (systemMode == MODE_RUN)
    {
        if (thisTick - lastUpdate >= 1000)
        {
            lastUpdate = thisTick;
            if (systemDirection == MODE_UP)
            {
                totalSeconds++;
            }
            if (systemDirection == MODE_DOWN)
            {
                if (totalSeconds > 0)
                {
                    totalSeconds--;
                }
                else
                {
                    myma.disableFlashColon();
                    systemMode = MODE_ALERT;
                    systemDirection = MODE_UP;
                    Serial.println(t_alert);
                }
            }
        }
    }
}

void doAlertEffect1()
{
    static int16_t x[] = {0, 0, 0, 0};
    static int16_t y[] = {0, 0, 0, 0};
    static int16_t r[] = {0, 0, 0, 0};
    static int16_t l[] = {0, 0, 0, 0};

    myma.Fill(0);

    for (byte i = 0; i < 4; i++)
    {
        if (r[i] >= l[i])
        {
            randomSeed(micros());
            r[i] = 1;
            x[i] = random(myma.matrix->width());
            y[i] = random(myma.matrix->height());
            l[i] = (x[i] + x[i] > myma.matrix->width() ? x[i] : myma.matrix->width() - x[i]) / 2;
        }
        else
        {
            r[i] = r[i] + 1;
        }
        myma.matrix->drawCircle(x[i], y[i], r[i], 1);
    }

    myma.display();
}