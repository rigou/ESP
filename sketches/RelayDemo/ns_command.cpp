#include <rgWiFi.h>
#include "main.h"
#include "ns_common.h"
#include "ns_command.h"
#include "ns_network.h"

extern Feature_struct Features;
extern NetInfo NetInfo_struct;

/* Add your own code here ****************************************************/

// cmd_Exec() calls this function to execute user-defined commands, if any
static int cmd_exec_usr(Command cmd_int, int value_int, char *net_reply_str) {
	rgTrace(__func__);
    int retval_int=0; // success

    strcpy(net_reply_str, "ERR 3 "); // invalid sequence
	cmd_get_string(net_reply_str+6, cmd_int, value_int);
	retval_int=1;
	
	//Serial.printf("cmd_exec_usr() returns %d\n", retval_int);
	return retval_int;
}

/* Do not change anything after this line ************************************/

/* Execute given command with given value
*  cmd_int: any of the CMD_* constants
*  see CMD_HELP in ns_command.h for a list of all commands
*/
int cmd_Exec(Command cmd_int, int value_int) {
	rgTrace(__func__);
	//Serial.printf("cmd_exec %d, value %d\n", cmd_int, value_int);
	int retval_int=0; // success
	char net_reply_str[REPLY_MAXLEN+1];
	char timestamp_str[11];
	const int RESET_DELAY=5; // seconds
	IPAddress ip_obj=WiFi.localIP();
    String mac_obj=WiFi.macAddress();
                    
	com_Uptime(timestamp_str);
	switch (cmd_int) {
		case CMD_ADMIN: 
			switch (value_int) {
				case 0:
					strncpy(net_reply_str, CMD_HELP, sizeof(net_reply_str));
					break;
				case 1:
					sprintf(net_reply_str, "%s %s\n%s %s", APP_NAME, APP_VERSION, NETSERVER_NAME, NETSERVER_VERSION);
					break;
				case 2:
					sprintf(net_reply_str, "%s %s", NetInfo_struct.net_hostname, timestamp_str);
					break;
				case 3:
                    sprintf(net_reply_str, "%d.%d.%d.%d:%d %s\n", ip_obj[0], ip_obj[1], ip_obj[2], ip_obj[3], SERVER_PORT, mac_obj.c_str());
					break;
                case 4:
					sprintf(net_reply_str, "Run %d Err %d", Features.Runled_enabled_int, Features.Errled_enabled_int);
					break;
                case 5:
					sprintf(net_reply_str, "OTA %d", Features.Ota_enabled_int);
					break;

				case 99:
					sprintf(net_reply_str, "Reset in %d seconds", RESET_DELAY);
					net_Reply(net_reply_str);
					net_CloseClient();
					delay(RESET_DELAY*1000);
					ESP.restart();
					// this point is never reched
					break;

				default:
					strcpy(net_reply_str, "ERR 2 "); // invalid value
					cmd_get_string(net_reply_str+6, cmd_int, value_int);
					retval_int=1;
					break;
			}
			net_Reply(net_reply_str);
			break;
		
		case CMD_FEATURE:
			switch (value_int) {
				case 0:
					com_RunLed(LOW);
					Features.Runled_enabled_int=0;
                    break;
				case 1:
					Features.Runled_enabled_int=1;
					break;
				case 2:
					com_ErrLed(LOW);
					Features.Errled_enabled_int=0;
                    break;
				case 3:
					Features.Errled_enabled_int=1;
					break;

				default:
					strcpy(net_reply_str, "ERR 2 "); // invalid value
					cmd_get_string(net_reply_str+6, cmd_int, value_int);
					retval_int=1;
					break;
			}
			if (retval_int==0) {
				strcpy(net_reply_str, "OK ");
				cmd_get_string(net_reply_str+3, cmd_int, value_int);
			}
			net_Reply(net_reply_str);
			break;

        case CMD_GET_GPIO:
            cmd_get_string(net_reply_str, cmd_int, value_int);
            sprintf(net_reply_str+3, " %d", digitalRead(value_int));
            net_Reply(net_reply_str);
            break;

        case CMD_SET_HIGH:
        case CMD_SET_LOW:
            digitalWrite(value_int, cmd_int==CMD_SET_HIGH?HIGH:LOW);
            strcpy(net_reply_str, "OK ");
			cmd_get_string(net_reply_str+3, cmd_int, value_int);
			net_Reply(net_reply_str);
			break;

		case CMD_WAIT:
			rgwifi_OtaDelay(value_int*1000);
			strcpy(net_reply_str, "OK ");
			cmd_get_string(net_reply_str+3, cmd_int, value_int);
			net_Reply(net_reply_str);
			break;

        default:
            retval_int=cmd_exec_usr(cmd_int, value_int, net_reply_str);
			net_Reply(net_reply_str);
            break;
	}

	//Serial.printf("cmd_Exec() returns %d\n", retval_int);
	return retval_int;
}

// read a command string from network (non-blocking) and parse it into given array of CmdToken
// return value: number of tokens parsed, 0=nothing read from network, -1=invalid command
int cmd_Read(CmdToken *cmd_tokens) {
	rgTrace(__func__);
	int retval_int=0;
	char buffer_str[CMD_STRING_SIZE+1];
	int cmd_length_int=0;
	// read command string from network (non blocking)
	cmd_length_int=net_Receive(buffer_str, CMD_STRING_SIZE+1);
	if (cmd_length_int > 0) {
		// parse command(s) and value(s) contained in the command string
		for (int idx_int=0; idx_int<CMD_TOKENS_MAX; idx_int++) {
			cmd_tokens[idx_int].cmd_int=CMD_WAIT;
			cmd_tokens[idx_int].val_int=0;
		}
		int ntokens_int=cmd_parse(buffer_str, cmd_tokens);
		if (ntokens_int>0) {
			retval_int=ntokens_int;
		}
		else {
			char net_reply_str[6+CMD_STRING_SIZE+1];
			strcpy(net_reply_str, "ERR 1 "); // parse error
			strcat(net_reply_str, buffer_str); 
			net_Reply(net_reply_str);
			retval_int=-1;
		}
	}
	return retval_int;
}

/* A command string is a series of concatenated 3-chars tokens
	3-char token: Cnn	C=command letter nn=2 digits value (00-99)
	Only the first CMD_TOKENS_MAX will be returned, exceeding tokens will be silently ignored
	Return value: number of tokens parsed in cmd_tokens[] or 0 on error (invalid token letter)
*/
static int cmd_parse(const char *commands_str, struct CmdToken *cmd_tokens) {
	rgTrace(__func__);
	int error_int=1;
	int bufflen_int=strlen(commands_str);
	int count_int=0;
	if (bufflen_int>0 && bufflen_int % 3 == 0) {
		int offset_int=0;
		for (offset_int=0; offset_int<bufflen_int; offset_int+=3) {
			char token_str[4];
			memcpy(token_str, commands_str+offset_int, 3);
			token_str[3]='\0';
            for (int idx_int=0; idx_int<sizeof(CMD_LETTERS); idx_int++) {
                if (token_str[0] == CMD_LETTERS[idx_int]) {
                    cmd_tokens[count_int].cmd_int=(Command)idx_int;
                    cmd_tokens[count_int++].val_int=atoi(token_str+1);
                    error_int=0;
                    break;
                }
                else
                    error_int=1;
            }
			if (error_int) // abort parsing on 1st error
				break;
		}
	}
	return (error_int==0?count_int:0);
}

// return the 3 chars command string corresponding to cmd_int and value_int
static char *cmd_get_string(char *buffer_str, Command cmd_int, int value_int) {
	rgTrace(__func__);
	buffer_str[0]=CMD_LETTERS[cmd_int];
	sprintf(buffer_str+1, "%02d", value_int);
	buffer_str[3]='\0';
	return buffer_str;
}
