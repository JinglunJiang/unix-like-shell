#ifndef _SIGNAL_HANDLERS_H_
#define _SIGNAL_HANDLERS_H_

#include "shell.h"

extern volatile pid_t foreground_pid;

void initialize_signal_handlers();

#endif