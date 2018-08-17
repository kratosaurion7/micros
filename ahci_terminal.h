#ifndef AHCI_TERM_H
#define AHCI_TERM_H

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "keyboard.h"
#include "framebuffer.h"
#include "ahci_driver.h"

enum ahci_terminal_states
{
    MAIN_SCREEN,
    PORT_SCREEN,
    COMMAND_SCREEN
};

enum ahci_terminal_states current_state;
uint8_t view_port_nb;
uint8_t view_command_nb;

#define CMD_MAXLEN 30
int commandLineIndex;
char commandLineEntry[CMD_MAXLEN];
BOOL command_latch;
BOOL cmdredraw;

struct ahci_host_regs* previous_host;
struct ahci_port_regs* previous_ports;

// int displayWidth;
// int displayHeight;
// char** display;

void init_ahci_term();

void ahci_term_update();
void ahci_term_drawdisplay();

void ahci_term_task();
void ahci_term_kbhook(keyevent_info* info);

void ahci_term_parse_cmd(const char* cmdline);

void ahci_term_drawoverlay();
void ahci_term_drawoverlay_main();
void ahci_term_drawoverlay_port();
void ahci_term_drawoverlay_command();

#endif