#pragma once

#include <Arduino.h>
#include <BluetoothSerial.h>

//static void bt_event_handler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

bool bt_Begin(const char *bt_id);
void bt_End(void);
size_t bt_Read(char *buffer_out, size_t buffer_size);
size_t bt_Writeln(const char *buffer);
size_t bt_Printf(const char *fmt, ... );
