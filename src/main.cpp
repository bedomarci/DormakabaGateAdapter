#include <Arduino.h>
#include <JC_Button.h>
#include <TaskScheduler.h>

#define PIN_GATE_INPUT 2
#define PIN_NYX_INPUT 3
#define PIN_NYX_OUTPUT 5
#define PIN_GATE_OUTPUT 4

#define FORWARDING_PAUSE 15000
#define BLINK_SPEED 100

void gateCounterForwardEnable();

void doBlink();

void blink(int num);

bool gateCounterEnabled = true;
bool ledState = false;
//Button(pin, dbTime, puEnable, invert);
Button gateCounter(PIN_GATE_INPUT, 50, false, true);
Button nayaxTrigger(PIN_NYX_INPUT, 50, false, true);

Scheduler runner;
Task tGateCounterForwardEnable(TASK_IMMEDIATE, TASK_ONCE, &gateCounterForwardEnable, &runner, true);
Task tBlink(BLINK_SPEED, 2, &doBlink, &runner, false);


void setup() {
    Serial.begin(115200);
    gateCounter.begin();
    nayaxTrigger.begin();
    pinMode(PIN_NYX_OUTPUT, OUTPUT);
    pinMode(PIN_GATE_OUTPUT, OUTPUT);
    blink (5);
}

void loop() {
    runner.execute();
    gateCounter.read();
    nayaxTrigger.read();

    if (gateCounterEnabled) {
        digitalWrite(PIN_NYX_OUTPUT, gateCounter.isPressed());
//        Serial.println(gateCounter.isPressed());
        if (gateCounter.wasPressed()) {
            Serial.println("Coin triggered gate.");
            blink(2);
        }
    } else {
        //Az elso atbillenesnel ujra engedelyezzuk a counter tovabbitast
        if (gateCounter.wasReleased()) {
            gateCounterEnabled = true;
            tGateCounterForwardEnable.disable();
            Serial.println("BLOCK OFF");
        }
    }


    if (nayaxTrigger.wasPressed() || nayaxTrigger.wasReleased()) {
        if (nayaxTrigger.wasPressed()) {
            Serial.println("Nayax triggered gate");
            blink(3);
        }

        digitalWrite(PIN_GATE_OUTPUT, nayaxTrigger.isPressed());
        if (nayaxTrigger.isPressed()) {
            Serial.println("BLOCK ON");
            gateCounterEnabled = false;
            tGateCounterForwardEnable.restartDelayed(FORWARDING_PAUSE);
        }
    }
}

void gateCounterForwardEnable() {
    //Ha 15 mpig semmi nem tortenik, ujra engedelyezzuk a jeltovabbitast
    gateCounterEnabled = true;
    Serial.println("BLOCK OFF");
}

void blink(int num) {
    ledState = false;
    tBlink.disable();
    tBlink.setIterations(num * 2);
    tBlink.restart();
}

void doBlink() {
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
}