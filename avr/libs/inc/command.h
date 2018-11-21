#ifndef _DEV_COMMAND_H
#define _DEV_COMMAND_H

#include <stdbool.h>

#if !defined(COMMAND_INPUT_ARG_MAX)
#define COMMAND_INPUT_ARG_MAX 16
#endif

#if !defined(COMMAND_CMD_PREFIX)
#define COMMAND_CMD_PREFIX "CMD"
#endif

/** 
 * Serial Command option list.
 *
 * New command can be added to the list and the opt if found by the parse will call the given vector.
 * hasArg, if true, will force the next opt to be an optarg. They are mandatory but not checked to exist.
 *    If an optarg is missing, the next token or null will be taken as the optarg even if it is an opt.
 */
struct Command {
  char * opt;
  void (*vector)(char *);
  bool hasArg;
};

void command_setup(struct Command optList[], size_t size);
int command_execute(char * input, void (*echoVector)(char *, char *));

#endif /* _DEV_COMMAND_H */
