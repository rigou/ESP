#pragma once

#define OTALIB_NAME "rgOTA" // spaces not permitted
#define OTALIB_VERSION "v1.0.0"

void OTASetup(const char* hostname_str, const char* ota_password_hash_str);
void OTADelay(unsigned long millisec_lng);
