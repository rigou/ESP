#pragma once

#define CMD_STRING_SIZE 30
#define CMD_TOKENS_MAX (CMD_STRING_SIZE/3)

// any change made to this enum should be reflected in CMD_LETTERS[]
enum Command {
	CMD_ADMIN,		// administrative commands
	CMD_FEATURE,	// enable/disable features
    CMD_GET_GPIO,	// GPIO control
    CMD_SET_HIGH,
    CMD_SET_LOW,
	CMD_WAIT		// wait given number of seconds

    // Add your own command tokens here, if any
    // ...
};
const char CMD_LETTERS[]="AFGHLW"; // same order as enumeration "Command"

// if you do not want to implement this rather long help string then set REPLY_MAXLEN=30 below
// Add your own command help here, if any
const char CMD_HELP[]="A00  print help\n"
	"A01  print program name and version\n"
	"A02  print hostname and uptime\n"
	"A03  print IP and MAC\n"
	"A04  print Runled and ErrLed status (0=disabled, 1=enabled)\n"
	"A05  print OTA status (0=disabled, 1=enabled)\n"
	"A99  reset\n"
	"F00  disable Runled\n"
	"F01  enable RunLed\n"
	"F02  disable Errled\n"
	"F03  enable ErrLed\n"
	"Gxx  read gpio xx\n"
	"Hxx  set high gpio xx\n"
	"Lxx  set low gpio xx\n"
	"Wxx  wait xx seconds";

const unsigned int REPLY_MAXLEN=sizeof(CMD_HELP);
	
struct CmdToken {
	Command cmd_int;
	int val_int;
};

int cmd_Read(CmdToken *cmd_tokens);
int cmd_Exec(Command cmd_int, int value_int);

static int cmd_exec_usr(Command cmd_int, int value_int, char *net_reply_str);
static int cmd_parse(const char *commands_str, struct CmdToken *cmd_tokens);
static char *cmd_get_string(char *buffer_str, Command cmd_int, int value_int);
