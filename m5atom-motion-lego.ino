//Uncomment to compile trhough Arduino IDE
//#define ENABLE_ARDUINO_IDE_SUPPORT

#ifdef ENABLE_ARDUINO_IDE_SUPPORT
//#include <customLibs.h>
#else
//#include <sensor.h>
//#include <nbiot/nbiot.h>
#include "Arduino.h"
#endif

#include "M5Atom.h"
#include <AtomMotion.h>
#include <neotimer.h>
#include <ESP32Wiimote.h>
#include <Preferences.h>


#define LONG_PRESS 1000

AtomMotion Atom;
ESP32Wiimote wiimote;
Preferences preferences;


bool programButtonPressed = true;
bool motorDirection = false;
bool servoDirection = true;

bool waitingButtonCommand = true;
int runningProgram; // persisted in EEPROM (Flash)
boolean programStartedFlag = true;

Neotimer cycleTimer = Neotimer(1000);

int wiiMotor1 = 0;
int wiiMotor2 = 0;
bool wiiButtonAIsPressed = false;

void parametrizedCyclingProgram(int servoAngle1, int servoAngle2)
{
    if (cycleTimer.repeat())
    {
        servoDirection = !servoDirection;
        Serial.println("Servo direction toggled");
    }

    if (programButtonPressed)
    {
        motorDirection = !motorDirection;
        programButtonPressed = false; // clear flag
        Serial.println("Motor direction toggled");
    }

    if (motorDirection)
    {
        Atom.SetMotorSpeed(1, 127);
        Atom.SetMotorSpeed(2, 127);
    }
    else
    {
        Atom.SetMotorSpeed(1, -127);
        Atom.SetMotorSpeed(2, -127);
    }
    int servoAngle = servoDirection ? servoAngle2 : servoAngle1;
    Atom.SetServoAngle(1, servoAngle);
    Atom.SetServoAngle(2, servoAngle2-servoAngle);  // even servos goes the other direction
    Atom.SetServoAngle(3, servoAngle);
    Atom.SetServoAngle(4, servoAngle2-servoAngle);  // even servos goes the other direction
}

void program0Loop()
{
    M5.dis.drawpix(0, 0x00ff00); // green
    parametrizedCyclingProgram(0, 180);
}

void program1Loop()
{
    M5.dis.drawpix(0, 0xffff00); // yellow
    parametrizedCyclingProgram(0, 90);
}

void program3WiimoteLoop()
{
    if (programStartedFlag)
    {
        // stop motors on program started (switched from another program)
        wiiMotor1 = 0;
        wiiMotor2 = 0;
        programStartedFlag = false;
        Serial.println("Wiimote program started");
    }

    M5.dis.drawpix(0, 0x0000ff); // blue

    // handle wiimote
    wiimote.task();
    if (wiimote.available() > 0) // new event (button change) received
    {
        ButtonState button = wiimote.getButtonState();
        // wheel motors
        if (button & BUTTON_RIGHT) // forward
        {
            wiiMotor1 = 1;
            wiiMotor2 = 1;
            if (button & BUTTON_ONE) // forward left
            {
                wiiMotor1 = 0;
                Serial.println("Forward left");
            }
            else if (button & BUTTON_TWO) // forward right
            {
                wiiMotor2 = 0;
                Serial.println("Forward right");
            }
            else
            {
                Serial.println("Forward");
            }
        }
        else if (button & BUTTON_LEFT) // reverse
        {
            wiiMotor1 = -1;
            wiiMotor2 = -1;
            if (button & BUTTON_ONE) // reverse left
            {
                wiiMotor1 = 0;
                Serial.println("Reverse left");
            }
            else if (button & BUTTON_TWO) // reverse right
            {
                wiiMotor2 = 0;
                Serial.println("Reverse right");
            }
            else
            {
                Serial.println("Reverse");
            }
        }
        else if (button & BUTTON_ONE) // pirouette left
        {
            wiiMotor1 = -1;
            wiiMotor2 = 1;
            Serial.println("Pirouette left");
        }
        else if (button & BUTTON_TWO) // pirouette right
        {
            wiiMotor1 = 1;
            wiiMotor2 = -1;
            Serial.println("Pirouette right");
        }
        else
        {
            wiiMotor1 = 0;
            wiiMotor2 = 0;
            Serial.println("Stop");
        }
        // manipulation servo
        if ((button & BUTTON_A) && !wiiButtonAIsPressed)    // A just pressed
        {
            servoDirection = !servoDirection;
            Serial.println("Servo direction toggled");
            wiiButtonAIsPressed = true;
        } else if (!(button & BUTTON_A)) {  // A released
            wiiButtonAIsPressed = false;
        }
    }
    Atom.SetMotorSpeed(1, 127 * wiiMotor1);
    Atom.SetMotorSpeed(2, 127 * wiiMotor2);
    int servoAngle = servoDirection ? 180 : 0;
    Atom.SetServoAngle(1, servoAngle);
    Atom.SetServoAngle(2, 180-servoAngle);  // even servos goes the other direction
    Atom.SetServoAngle(3, servoAngle);
    Atom.SetServoAngle(4, 180-servoAngle);  // even servos goes the other direction
}

void longPressCommand()
{
    // run next program
    runningProgram = (++runningProgram) % 3;
    preferences.putInt("runningProgram", runningProgram);
    programButtonPressed = false; // always clear command button flag in case the previous program has not clear it
    programStartedFlag = true;
}

void shortPressCommand()
{
    programButtonPressed = true;
}

void setup()
{
    preferences.begin("lego", false); 
    runningProgram = preferences.getInt("runningProgram", 0);

    M5.begin(true, false, true);
    Atom.Init(); // Motion I2C connectivity initialized here
    wiimote.init();
    wiimote.addFilter(ACTION_IGNORE, FILTER_ACCEL); // no accelerometer data needed (thus we receive much less events from wiimote)
}

void loop()
{
    // handle M5Atom button
    M5.update();
    if (waitingButtonCommand && M5.Btn.pressedFor(LONG_PRESS))
    { // is pressed (not released yet) for LONG_PRESS period
        Serial.println("Long press");
        longPressCommand();
        waitingButtonCommand = false; // wait for button release
    }
    else if (waitingButtonCommand && M5.Btn.wasReleased())
    { // release before LONG_PRESS period
        Serial.println("Short press");
        shortPressCommand();
    }
    else if (!waitingButtonCommand && M5.Btn.wasReleased())
    {
        Serial.println("Released after long press");
        waitingButtonCommand = true; // can accept next command (button press)
    }

    // run selected program loop
    switch (runningProgram)
    {
    case 0:
        program0Loop();
        break;

    case 1:
        program1Loop();
        break;

    case 2:
        program3WiimoteLoop();
        break;

    default:
        break;
    }
}