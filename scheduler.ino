/*
 * scheduler.ino
 *
 * This accessory contains a builtin-led on ESP8266
 * Setup code: 111-11-111
 * The Flash-Button(D3, GPIO0) on NodeMCU:
 *
 *  Created on: 2020-02-08
 *      Author: Mixiaoxiao (Wang Bin)
 *  Edited on: 2020-03-01
 *      Edited by: euler271 (Jonas Linn)
 *  Edited on: 2020-09-16
 *      Edited by: waaraawa
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <arduino_homekit_server.h>
#include <TaskScheduler.h>

#define PL(s) Serial.println(s)
#define P(s) Serial.print(s)

//D0 16 //led
//D3  0 //flash button
//D4  2 //led

#define PIN_LED 16//D0

const char *ssid = "your-ssid";
const char *password = "your-password";

void homekit_loop();
void homekit_status();

#define HOMEKIT_INTERVAL    200L // 200 ms
Scheduler ts;
Task tHomekitLoop(HOMEKIT_INTERVAL, TASK_FOREVER, &homekit_loop, &ts, true);

// Print heap and connection count every 5 sec continually.
Task tHomekitStatus(5000, TASK_FOREVER, &homekit_status, &ts, true);

void blink_led(int interval, int count) {
	for (int i = 0; i < count; i++) {
		builtinledSetStatus(true);
		delay(interval);
		builtinledSetStatus(false);
		delay(interval);
	}
}

void setup() {
	Serial.begin(115200);
	Serial.setRxBufferSize(32);
	Serial.setDebugOutput(false);

	pinMode(PIN_LED, OUTPUT);
	WiFi.mode(WIFI_STA);
	WiFi.persistent(false);
	WiFi.disconnect(false);
	WiFi.setAutoReconnect(true);
	WiFi.begin(ssid, password);

	printf("\n");
	printf("SketchSize: %d B\n", ESP.getSketchSize());
	printf("FreeSketchSpace: %d B\n", ESP.getFreeSketchSpace());
	printf("FlashChipSize: %d B\n", ESP.getFlashChipSize());
	printf("FlashChipRealSize: %d B\n", ESP.getFlashChipRealSize());
	printf("FlashChipSpeed: %d\n", ESP.getFlashChipSpeed());
	printf("SdkVersion: %s\n", ESP.getSdkVersion());
	printf("FullVersion: %s\n", ESP.getFullVersion().c_str());
	printf("CpuFreq: %dMHz\n", ESP.getCpuFreqMHz());
	printf("FreeHeap: %d B\n", ESP.getFreeHeap());
	printf("ResetInfo: %s\n", ESP.getResetInfo().c_str());
	printf("ResetReason: %s\n", ESP.getResetReason().c_str());
	DEBUG_HEAP();
	homekit_setup();
	DEBUG_HEAP();
	blink_led(200, 3);
}

void loop() {
    ts.execute();
//	homekit_loop();
//	delay(5);
}

void builtinledSetStatus(bool on) {
	digitalWrite(PIN_LED, on ? LOW : HIGH);
}

//==============================
// Homekit setup and loop
//==============================

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t name;
extern "C" void led_toggle();
extern "C" void accessory_init();

uint32_t next_heap_millis = 0;

void homekit_setup() {
	accessory_init();
	uint8_t mac[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(mac);
	int name_len = snprintf(NULL, 0, "%s_%02X%02X%02X", name.value.string_value, mac[3], mac[4], mac[5]);
	char *name_value = (char*)malloc(name_len + 1);
	snprintf(name_value, name_len + 1, "%s_%02X%02X%02X", name.value.string_value, mac[3], mac[4], mac[5]);
	name.value = HOMEKIT_STRING_CPP(name_value);

	arduino_homekit_setup(&config);
 
}

void homekit_loop() {
	arduino_homekit_loop();
}

void homekit_status() {
	INFO("heap: %d, sockets: %d", ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
}
