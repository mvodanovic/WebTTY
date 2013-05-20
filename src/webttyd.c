#include <stdio.h>
#include <stdlib.h>
#include <libssh/libssh.h>
#include <unistd.h>
#include <string.h>

#include "verify_knownhost.h"
#include "execute_remote_cmd.h"

int main (int argc, char **argv)
{
	ssh_session session;
	ssh_channel channel;
	int rc;
	int verbosity = SSH_LOG_NOLOG;
	int port = 22;
	char *password;
	char *cmd = NULL;
	int i;
	char buffer[1025];

	if (argc <= 1) {
		exit(EXIT_FAILURE);
	}
	
	for (i = 1; i < argc; i++) {
		if (cmd == NULL) {
			cmd = (char *) malloc((strlen(argv[i]) + 1) * sizeof(char));
			strcpy(cmd, argv[i]);
		} else {
			cmd = (char *) realloc(cmd, (strlen(argv[i]) + strlen(cmd) + 2) * sizeof(char));
			strcat(cmd, " ");
			strcat(cmd, argv[i]);
		}
		
	}

	/// Open session & set options
	session = ssh_new();
	if (session == NULL) {
		free(cmd);
		exit(EXIT_FAILURE);
	}
	ssh_options_set(session, SSH_OPTIONS_HOST, "localhost");
	ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_options_set(session, SSH_OPTIONS_PORT, &port);

	/// Connect to server
	rc = ssh_connect(session);
	if (rc != SSH_OK) {
		fprintf(stderr, "Error connection to localhost: %s\n", ssh_get_error(session));
		ssh_free(session);
		free(cmd);
		exit(EXIT_FAILURE);
	}

	/// Verify the server's identity
	if (verify_knownhost(session) < 0) {
		ssh_disconnect(session);
		ssh_free(session);
		free(cmd);
		exit(EXIT_FAILURE);
	}

	/// Authenticate ourselves
	password = getpass("Password: ");
	rc = ssh_userauth_password(session, NULL, password);
	if (rc != SSH_AUTH_SUCCESS) {
		fprintf(stderr, "Error authenticating with password: %s\n", ssh_get_error(session));
		ssh_disconnect(session);
		ssh_free(session);
		free(cmd);
		exit(EXIT_FAILURE);
	}

	/// Open a channel
	channel = ssh_channel_new(session);
	if (channel == NULL) {
		fprintf(stderr, "Couldn't open a channel: %s\n", ssh_get_error(session));
		ssh_disconnect(session);
		ssh_free(session);
		free(cmd);
		exit(EXIT_FAILURE);
	}

	/// Set channel blocking/nonblocking
	ssh_channel_set_blocking(channel, 1);

	/// Open a channel session
	rc = ssh_channel_open_session(channel);
	if (rc != SSH_OK) {
                fprintf(stderr, "Couldn't open a channel session: %s\n", ssh_get_error(session));
                ssh_channel_close(channel);
                ssh_channel_free(channel);
                ssh_disconnect(session);
                ssh_free(session);
                free(cmd);
                exit(EXIT_FAILURE);
        }

	/// Request a TTY
        rc = ssh_channel_request_pty_size(channel, "xterm-256color", 80, 25);
	if (rc != SSH_OK) {
		fprintf(stderr, "Couldn't get a TTY: %s\n", ssh_get_error(session));
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		ssh_disconnect(session);
		ssh_free(session);
		free(cmd);
		exit(EXIT_FAILURE);
	}

	/// Request a shell
	rc = ssh_channel_request_shell(channel);
	if (rc != SSH_OK) {
		fprintf(stderr, "Couldn't get a shell: %s\n", ssh_get_error(session));
		ssh_channel_close(channel);
                ssh_channel_free(channel);
                ssh_disconnect(session);
                ssh_free(session);
                free(cmd);
                exit(EXIT_FAILURE);
	}

	while (ssh_channel_read(channel, buffer, 1024, 0) > 0) {
		//ssh_channel_read_nonblocking(channel, buffer, 512, 0);
		printf("%s", buffer);
		sleep(5);
	}

	ssh_channel_write(channel, cmd, sizeof(cmd));

	while (ssh_channel_read(channel, buffer, 1024, 0) > 0) {
		//ssh_channel_read_nonblocking(channel, buffer, 512, 0);
                printf("%s", buffer);
		sleep(5);
        }

	//execute_remote_cmd(channel, cmd);

	ssh_channel_close(channel);
	ssh_channel_free(channel);
	ssh_disconnect(session);
	ssh_free(session);
	free(cmd);
	printf("Successful exit!\n");
	exit(EXIT_SUCCESS);
}
