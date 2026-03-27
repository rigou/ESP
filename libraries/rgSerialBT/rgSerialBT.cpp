#include "rgSerialBT.h"

/* use Serial bluetooth terminal Android app to connect to the MCU via BT
   https://www.kai-morich.de/android/
   see Doc_Linux/Bluetooth/Serial bluetooth terminal Android app
*/

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

// BluetoothSerial requires CPU Frequency >= 80 MHz
static BluetoothSerial SerialBT_obj;
bool bt_Connected=false; // true when a BT client is connected

// Board : ESP-WROOM-32E narrow Development Board, powered by its +5V Vin pin
// Measurement: current flowing through Vin
// cpu freq=80  disconnected or connected: 34-41 mA fluctuant
//              connecting, disconnecting or sending data: 105 mA
//              booting: 125 mA
// cpu freq=240 disconnected: 47-52 mA fluctuant
//              connected: 49-55 mA fluctuant
//              connecting, disconnecting or sending data: 122 mA
//              booting: 147 mA

// see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_spp.html
// https://github.com/espressif/esp-idf/blob/ef64e4e5b3/components/bt/host/bluedroid/api/include/api/esp_spp_api.h
  // typedef enum {
  //     ESP_SPP_INIT_EVT                    = 0,                /*!< When SPP is initialized, the event comes */
  //     ESP_SPP_UNINIT_EVT                  = 1,                /*!< When SPP is deinitialized, the event comes */
  //     ESP_SPP_DISCOVERY_COMP_EVT          = 8,                /*!< When SDP discovery complete, the event comes */
  //     ESP_SPP_OPEN_EVT                    = 26,               /*!< When SPP Client connection open, the event comes */
  //     ESP_SPP_CLOSE_EVT                   = 27,               /*!< When SPP connection closed, the event comes */
  //     ESP_SPP_START_EVT                   = 28,               /*!< When SPP server started, the event comes */
  //     ESP_SPP_CL_INIT_EVT                 = 29,               /*!< When SPP client initiated a connection, the event comes */
  //     ESP_SPP_DATA_IND_EVT                = 30,               /*!< When SPP connection received data, the event comes, only for ESP_SPP_MODE_CB */
  //     ESP_SPP_CONG_EVT                    = 31,               /*!< When SPP connection congestion status changed, the event comes, only for ESP_SPP_MODE_CB */
  //     ESP_SPP_WRITE_EVT                   = 33,               /*!< When SPP write operation completes, the event comes, only for ESP_SPP_MODE_CB */
  //     ESP_SPP_SRV_OPEN_EVT                = 34,               /*!< When SPP Server connection open, the event comes */
  //     ESP_SPP_SRV_STOP_EVT                = 35,               /*!< When SPP server stopped, the event comes */
  //     ESP_SPP_VFS_REGISTER_EVT            = 36,               /*!< When SPP VFS register, the event comes */
  //     ESP_SPP_VFS_UNREGISTER_EVT          = 37,               /*!< When SPP VFS unregister, the event comes */
  // } esp_spp_cb_event_t;
static void bt_event_handler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  switch (event) {
    case ESP_SPP_SRV_OPEN_EVT:
      bt_Connected=true;
      break;
    case ESP_SPP_CLOSE_EVT:
      bt_Connected=false;
      break;
    default:
      break;
  }
}

// set the Bluetooth device name and activate BT
// return value: true=success, false=error
bool bt_Begin(const char *bt_id) {
  bool retval=false;
  SerialBT_obj.register_callback(bt_event_handler);
  retval=SerialBT_obj.begin(bt_id);
  if (retval)
    SerialBT_obj.setTimeout(100); // milliseconds
  return retval;
}

void bt_End(void) {
  SerialBT_obj.end();
}

// read data received over BT until \n is found, and replace this \n with \0
// return value: number of characters stored in buffer_out, not counting the final \0
size_t bt_Read(char *buffer_out, size_t buffer_size) {
  size_t retval=0;
  if (bt_Connected && SerialBT_obj.available())
    retval=SerialBT_obj.readBytesUntil('\n', buffer_out, buffer_size-1);
  buffer_out[retval]='\0';
  return retval;
}

// print a null-terminated string over BT and append a \n
// returns the number of bytes written
size_t bt_Writeln(const char *buffer) {
  size_t retval=0;
  if (bt_Connected)
    retval=SerialBT_obj.printf("%s\n", buffer);
  return retval;
}

// Send given variable list of args over BT using a printf-like format string
// code dderived from example at https://en.cppreference.com/w/c/io/vfprintf
size_t bt_Printf(const char *fmt, ... ) {
	va_list args1;
    va_start(args1, fmt);
    va_list args2;
    va_copy(args2, args1);
    char buf[vsnprintf(NULL, 0, fmt, args1)+1];
    va_end(args1);
    vsnprintf(buf, sizeof buf, fmt, args2);
    va_end(args2);
    return SerialBT_obj.write((uint8_t*)buf, sizeof buf-1);
}
