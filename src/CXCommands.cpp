// ETH32nix - an Enemy Territory cheat for Linux
// Copyright (c) 2009 eth32 team
// www.nixcoders.org

// kobject: the idea is to put our own console commands in here, and
// leave hooked engine/cgame commands in CEngine


// Member functions are from Mars, ordinary functions are from Venus.

#include "eth32.h"
#include "settings.h"

#ifdef ETH32_PRIV
	#include "../priv/priv.h"
#endif

CXCommands CCommands;
extern void *etpro_A;
extern void *etpro_B;
extern void *etpro_C;
extern void *etpro_E;
extern uint32 etpro_win32;
extern uint32 etpro_linux;

void xcmd_RandName()
{
	eth32.settings.doNamesteal = false;
	if (!Tools.nNames) {
		Syscall.UI_Print("^3No player names loaded\n");
		return;
	}

	int n = (int)(random()*Tools.nNames);
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND,(va("name \"%s\"\n", Tools.nameList[n]))); 
}

void xcmd_RandGuid()
{
	if (!Tools.nGuids) {
		Syscall.UI_Print("^3No guids loaded!\n");
		return;
	}

	int n = (int)(random()*Tools.nGuids);
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND,(va("eth_set etproguid \"%s\"\n", Tools.guidList[n])));
}

void xcmd_RandMac()
{
	if (!Tools.nMacs) {
		Syscall.UI_Print("^3No jaymacs loaded!\n");
		return;
	}

	int n = (int)(random()*Tools.nMacs);
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND,(va("eth_set jaymac \"%s\"\n", Tools.macList[n])));
}

void xcmd_unbanCommand()
{
	system("rm -rf ~/.etwolf/etmain/etkey");
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "pb_cdkeyreg; wait 50 \n");
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "eth_randguid; eth_randmac; wait 50; eth_randname; reconnect \n");
}

void xcmd_setEtproGuid()
{
	if (Syscall.UI_Argc() == 1)
	{
		if (!eth32.settings.etproGuid || strlen(eth32.settings.etproGuid) < 2)
			Syscall.UI_Print("^dEtpro Guid: ^2original");
		else
			Syscall.UI_Print("^dEtpro Guid: ^7%s", eth32.settings.etproGuid);
		return;
	}

	strncpy(eth32.settings.etproGuid, Syscall.UI_Argv(1), 1024);
	Syscall.UI_Print("^dNew etpro guid: ^7%s\n^dSetting won't take effect until next map (or do /reconnect)!", eth32.settings.etproGuid);
}

//eth_spam
void ethSpam()
{
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "say \"^0        __  .__    ________  ________        ^o.__\"\n");
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "say \"^0  _____/  |_|  |__ \\_____  \\ \\_____  \\  ^o____ |__|__  ___\"\n");
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "say \"^0_/ __ \\   __\\  |  \\  _(__  <  /  ____/ ^o/    \\|  \\  \\/  /\"\n");
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "say \"^0\\  ___/|  | |   Y  \\/       \\/       \\^o|   |  \\  |>    <\"\n");
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "say \"^0 \\___  >__| |___|  /______  /\\_______ \\^o___|  /__/__/\\_ \\\"\n");
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "say \"^0     \\/          \\/       \\/         \\/    ^o\\/         \\/\"\n");
}
 void noobspamCommand() {

	 orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "say \"^3     __            _        \"\n");
	 orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "say \"^3  /\\ \\ \\___   ___ | |__\"\n");
	 orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "say \"^3 /  \\/ / _ \\ / _ \\| '_ \\\"\n");
	 orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "say \"^3/ /\\  / (_) | (_) | |_) |\"\n");
	 orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "say \"^3\\_\\ \\/ \\___/ \\___/|_.__/\"\n");
 }

void setEtproOS()
{
	if (Syscall.UI_Argc() == 1){
		Syscall.UI_Print("^2%s", eth32.settings.etproOs ? "linux" : "win32");
		return;
	}

	char *cmd = strdup(Syscall.UI_Argv(1));

	if (!strcasecmp(cmd, "win"))
		eth32.settings.etproOs = false;
	else
		eth32.settings.etproOs = true;

	Syscall.UI_Print("^2new etpro OS set to %s", eth32.settings.etproOs ? "linux" : "win32");
	free(cmd);

	if (eth32.cgMod->type == MOD_ETPRO)
		orig_etproAntiCheat(etpro_A, etpro_B, etpro_C, eth32.settings.etproOs ? etpro_linux : etpro_win32, etpro_E, eth32.settings.etproGuid);
}

CXCommands::CXCommands(void)
{
	cmdsRegistered = false;
}

void CXCommands::RegisterCommands()
{
	if (cmdsRegistered || !orig_Cmd_AddCommand)
		return;

	orig_Cmd_AddCommand("eth_randname", &xcmd_RandName);
	orig_Cmd_AddCommand("eth_unban", &xcmd_unbanCommand);
	orig_Cmd_AddCommand("eth_randguid", &xcmd_RandGuid);
	orig_Cmd_AddCommand("eth_randmac", &xcmd_RandMac);
	orig_Cmd_AddCommand("eth_etproos", &setEtproOS);
	orig_Cmd_AddCommand("eth_etproguid", &xcmd_setEtproGuid);
	// spam
	orig_Cmd_AddCommand("eth_spam", &ethSpam);
    orig_Cmd_AddCommand("eth_nspam", &noobspamCommand);


	#ifdef ETH32_PRIV
		RegisterPrivCommands();
	#endif

	cmdsRegistered = true;
}
