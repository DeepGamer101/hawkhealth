/* HawkHealth — Week 3 concept sketch (Wokwi, ST Nucleo-C031C6).
 *
 * THE SUPER-LOOP, AND WHY IT DOESN'T SCALE. No RTOS this week -- just setup() and one loop()
 * running every job in sequence. Watch three things go wrong at once:
 *
 *   1) Two blinkers can't hold independent rates -- their delay()s add up (250 + 750 = 1000).
 *   2) A CRITICAL job that MUST run every 100 ms misses its deadline by ~10x -- it can only
 *      run once per trip through loop(), which is ~1000 ms. In a bedside monitor that's a
 *      missed alarm.
 *   3) One occasionally-slow "sensor read" makes ALL the timing JITTER unpredictably (the
 *      critical gap jumps from ~1000 ms to ~1400 ms whenever the slow read happens).
 *
 * With two jobs you can do the arithmetic. Add a hard deadline and a variable-time job and you
 * already can't predict when anything runs -- and real systems have many jobs. Decoupling all of
 * this is exactly what an RTOS scheduler does for you (Week 5).
 *
 * DIAGRAM: same as before -- an LED on PB1 (anode -> PB1, cathode -> GND). No library needed.
 */
#define LED_A LED_BUILTIN   /* on-board LED (PA5) -- "wants" 250 ms */
#define LED_B PB1           /* external LED       -- "wants" 750 ms */

unsigned long lastCritical = 0;
unsigned long iter = 0;

/* A job that MUST run every 100 ms -- think: check whether a vital crossed a critical threshold. */
void checkCritical() {
    unsigned long now = millis();
    Serial.print("  !! CRITICAL check @ ");
    Serial.print(now);
    Serial.print(" ms   (gap since last = ");
    Serial.print(now - lastCritical);
    Serial.println(" ms, target 100)");
    lastCritical = now;
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_A, OUTPUT);
    pinMode(LED_B, OUTPUT);
    lastCritical = millis();
}

void loop() {
    iter++;

    checkCritical();                 /* wants every 100 ms... but only gets here once per loop */

    /* Job A: blink (wants 250 ms) */
    digitalWrite(LED_A, !digitalRead(LED_A));
    Serial.print("A @ "); Serial.print(millis()); Serial.println(" ms");
    delay(250);

    /* "Sensor read": usually instant, but every 4th pass it's slow (400 ms) -> jitter */
    if (iter % 4 == 0) {
        Serial.println("  (slow sensor read this pass...)");
        delay(400);
    }

    /* Job B: blink (wants 750 ms) */
    digitalWrite(LED_B, !digitalRead(LED_B));
    Serial.print("B @ "); Serial.print(millis()); Serial.println(" ms");
    delay(750);
}
