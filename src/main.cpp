#include <Arduino.h>
#include <JC_Button.h>
#include <TaskScheduler.h>

#define PIN_GATE_INPUT 2
#define PIN_NYX_INPUT 3
#define PIN_NYX_OUTPUT 5
#define PIN_GATE_OUTPUT 4
#define PIN_SYSTEM_LED 6

#define FORWARDING_PAUSE 15000
#define BLINK_SPEED 150

void gateCounterForwardEnable();

void doBlink();

void blink(int num);

void breathe();

bool gateCounterEnabled = true;
bool ledState = false;
Button gateCounter(PIN_GATE_INPUT, 50, false, true);
Button nayaxTrigger(PIN_NYX_INPUT, 50, false, true);

Scheduler runner;
Task tGateCounterForwardEnable(TASK_IMMEDIATE, TASK_ONCE, &gateCounterForwardEnable, &runner, true);
Task tBlink(BLINK_SPEED, 2, &doBlink, &runner, false);
Task tBreathe(15, TASK_FOREVER, &breathe, &runner, true);


void setup() {
    Serial.begin(115200);
    gateCounter.begin();
    nayaxTrigger.begin();
    Serial.println("====================================================");
//    Serial.println("| This is a Nayax card acceptor bypass board.      |");
//    Serial.println("| Designed for Dormakaba gate extension.           |");
    Serial.println("| Circuit and program design by Marci Bedo.        |");
    Serial.println("| 2021, Budapest, Hungary                          |");
//    Serial.println("====================================================");
//    Serial.println("| Custom electronic designs and manufacturing:     |");
    Serial.println("| https://orangepixel.hu                           |");
    Serial.println("| hardware@orangepixel.hu                          |");
    Serial.println("====================================================");
    pinMode(PIN_NYX_OUTPUT, OUTPUT);
    pinMode(PIN_GATE_OUTPUT, OUTPUT);
    pinMode(PIN_SYSTEM_LED, OUTPUT);
    blink(5);
}

void loop() {
    runner.execute();
    gateCounter.read();
    nayaxTrigger.read();

    if (gateCounterEnabled) {
        digitalWrite(PIN_NYX_OUTPUT, gateCounter.isPressed());
        if (gateCounter.wasPressed()) {
            Serial.println("Coin acceptor triggered the gate.");
            blink(2);
        }
    } else {
        //Az elso atbillenesnel ujra engedelyezzuk a counter tovabbitast
        if (gateCounter.wasReleased()) {
            gateCounterEnabled = true;
            tGateCounterForwardEnable.disable();
            Serial.println("Unblocking Nazax return.");
        }
    }


    if (nayaxTrigger.wasPressed() || nayaxTrigger.wasReleased()) {
        if (nayaxTrigger.wasPressed()) {
            Serial.println("Nayax triggered the gate.");
            blink(3);
        }

        digitalWrite(PIN_GATE_OUTPUT, nayaxTrigger.isPressed());
        if (nayaxTrigger.isPressed()) {
            Serial.println("Blocking Nayax return on CH2.");
            gateCounterEnabled = false;
            tGateCounterForwardEnable.restartDelayed(FORWARDING_PAUSE);
        }
    }
}

void gateCounterForwardEnable() {
    //Ha 15 mpig semmi nem tortenik, ujra engedelyezzuk a jeltovabbitast
    if (!gateCounterEnabled) {
        gateCounterEnabled = true;
        Serial.println("Unblocking Nayax return after being idle for 15 seconds.");
    }
}

void blink(int num) {
    tBreathe.disable();
    ledState = false;
    tBlink.disable();
    digitalWrite(PIN_SYSTEM_LED, ledState);
    tBlink.setIterations(num * 2);
    tBlink.restartDelayed(200);
}

void doBlink() {
    if (tBlink.isLastIteration()) {
        tBreathe.restartDelayed(TASK_SECOND * 2);
    }
    ledState = !ledState;
    digitalWrite(PIN_SYSTEM_LED, ledState);
}

void breathe() {
    int step = tBreathe.getRunCounter() % 256;
    step = constrain(step, 0, 127);
    int pwm = (step < 64) ? step : 63 - abs(64 - step);
    analogWrite(PIN_SYSTEM_LED, pwm);
}