#include <string.h>
#include "command.h"

static struct Command * optList;
static size_t optSize;

/**
 * Parse the given input line and executes any command found command.
 * 
 * New command and function vector should be set in the global optList.
 * 
 * @param input null-terminated string
 * @param echoVector Callback with argument (cmd name, cmd args)
 * @return 0 if no cmd prexis, 1 if something was executed
 * 
 */
int command_execute(char * input, void (*echoVector)(char *, char *)) {
	char * argv[COMMAND_INPUT_ARG_MAX];
	size_t argc = 0;

	// Check for cmd prefix
	if (strstr(input, COMMAND_CMD_PREFIX) != input) {
		return -1;
	}

	char * token = strtok(input, " ");
	while(token != NULL) {
		if (argc < COMMAND_INPUT_ARG_MAX) {
			argv[argc] = token;
			argc++;
		}
		token = strtok(NULL, " ");
	}

	int optind;
	for (optind = 1; optind < argc; optind++) {
		char * optArg = NULL;
		int optDefInd = -1;
		if (*argv[optind] == '-') {
			for (int i = 0; i < optSize; i++) {
				if (strcmp(optList[i].opt, argv[optind] + 1) == 0) {
					if (optList[i].hasArg && i < argc + 1) {
						optArg = argv[optind+1];
					}
					optDefInd = i;
					break;
				}
			}
		  
			if (optDefInd >= 0) {
				if (echoVector != NULL) {
					echoVector(optList[optDefInd].opt, optArg);
				}
				optList[optDefInd].vector(optArg);
				optind++; // skip the next token it was an arg
			}
		}
	}
	return 1;
}

/*
 * @see command.h
 */
void command_setup(struct Command opts[], size_t size) {
	optList = opts;
	optSize = size;
}
