#ifndef EXECUTE_REMOTE_CMD__H
#define EXECUTE_REMOTE_CMD__H

#include <libssh/libssh.h>

int execute_remote_cmd(ssh_session session, char *cmd);

#endif
