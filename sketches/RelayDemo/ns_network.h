#pragma once

#define SERVER_PORT 59000

void net_Setup();
int net_NetInfo();
void net_CloseClient();
int net_Receive(char *buffer_str, const int length_int);
void net_Reply(char *reply_str);

static int net_device_id();
