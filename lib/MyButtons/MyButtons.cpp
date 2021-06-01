#include "Arduino.h"
#include "MyButtons.h"

MyButtons::MyButtons(int buttonPin, int longTicks, int longerTicks, int deltaTicks)
{
    _buttonPin = buttonPin;
    _buttonLong = longTicks;
    _buttonLonger = longerTicks;
    _deltaTicks = deltaTicks;
    _waitingClear = 0;
}

void MyButtons::clearState() {
    _waitingClear = 1;
}

// returns the accumulates button press time and modifies the buttonMode status according to time and press/release
unsigned long MyButtons::check()
{

    static byte b_flow = 0;                 // used as debouncing shift register
    static unsigned long buttonPressed = 0; // internal number of ticks the button is being held down

    b_flow = b_flow << 1 | (digitalRead(_buttonPin) == LOW ? 1 : 0);

    if ((b_flow & 0b10011111) == 0b10011111 && _waitingClear == 0 )
    {
        buttonPressed = buttonPressed + _deltaTicks;
    }

    if( buttonPressed >= _buttonLong && _waitingClear == 0 ) {
        buttonMode = MYBUTTONS_HOLD_LONG;
    }

    if (buttonPressed >= _buttonLonger && _waitingClear == 0 )
    {
        buttonMode = MYBUTTONS_HOLD_LONGER;
    }

    if ((b_flow & 0b10011111) == 0) // Flow = Release; actual evaluation of type of press happens only on release time
    {
        buttonMode = MYBUTTONS_PRESS_NONE;

        if (buttonPressed > 0 && _waitingClear == 0 ) // waitingClear must not be active
        {

            if (buttonPressed < _buttonLong)
            {
                buttonMode = MYBUTTONS_RELEASE_SHORT;
            }
            else
            {
                if (buttonPressed >= _buttonLonger)
                {
                    buttonMode = MYBUTTONS_RELEASE_LONGER;
                }
                else
                {
                    buttonMode = MYBUTTONS_RELEASE_LONG;
                }
            }
        }
        if( _waitingClear == 1 && buttonPressed > 0 ) { // a button was released (and ignored) since it was already in waitingClear state. Reset waitingClear now.
            _waitingClear = 0;
        }
        buttonPressed = 0;
    }

    return buttonPressed;
}

// button was just released after being held down for time < longTicks
bool MyButtons::release_short()
{
    if (buttonMode == MYBUTTONS_RELEASE_SHORT)
    {
        return true;
    }

    return false;
}

// button was just released after being held down for longTicks < time < longerTicks
bool MyButtons::release_long()
{
    if (buttonMode == MYBUTTONS_RELEASE_LONG)
    {
        return true;
    }
    return false;
}

// button is still being held down for a time > longerTicks
bool MyButtons::hold_long()
{
    if (buttonMode == MYBUTTONS_HOLD_LONG )
    {
        return true;
    }
    return false;
}

// button is still being held down for a time > longerTicks
bool MyButtons::hold_longer()
{
    if (buttonMode == MYBUTTONS_HOLD_LONGER)
    {
        return true;
    }
    return false;
}

bool MyButtons::release_longer()
{
    if (buttonMode == MYBUTTONS_RELEASE_LONGER)
    {
        return true;
    }
    return false;
}