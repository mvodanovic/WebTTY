/// Standard library includes
#include <string>
#include <cerrno>
#include <libssh/libssh.h>
#include <libssh/callbacks.h>
#include <cstdlib>

/// Program-specific includes
#include "tty.h"
#include "logger.h"

void WebTTY::TTY::initParams(void)
{
	this->host = this->getParam("host");
	if (this->host.length() == 0) {
		this->host = "localhost";
	}

	this->port = atoi(this->getParam("port").c_str());
	if (this->port <= 0) {
		this->port = 22;
	}

	this->authType = this->getParam("authType");
	if (this->authType.length() == 0) {
		this->authType = "pubKey";
	}

	this->password = this->getParam("password");

	this->term = this->getParam("term");
	if (this->term.length() == 0) {
		this->term = "xterm-256color";
	}

	this->width = atoi(this->getParam("width").c_str());
	if (this->width <= 0) {
		this->width = 80;
	}

	this->height = atoi(this->getParam("height").c_str());
	if (this->height <= 0) {
		this->height = 25;
	}

	Logger::Log("host: ", 0); Logger::Log(this->host);
	Logger::Log("port: ", 0); Logger::Log(this->port);
	Logger::Log("authType: ", 0); Logger::Log(this->authType);
	Logger::Log("password: ", 0); Logger::Log(this->password);
	Logger::Log("term: ", 0); Logger::Log(this->term);
	Logger::Log("width: ", 0); Logger::Log(this->width);
	Logger::Log("height: ", 0); Logger::Log(this->height);
}

WebTTY::TTY::TTY(std::vector<std::string> &params)
{
	this->params = params;
	this->verbosity = SSH_LOG_NOLOG;
	this->active = 0;

	this->initParams();

	/// Setup SSH threading
	ssh_threads_set_callbacks(ssh_threads_get_noop());

	/// Initialize global cryptographic data structures
	int rc = ssh_init();
	if (rc != 0) {
		Logger::Die("TTY init failed!");
	}

	/// Open session & set options
	this->session = ssh_new();
	if (this->session == NULL) {
		ssh_finalize();
		Logger::Die("TTY session creation failed!");
	}
	ssh_options_set(this->session, SSH_OPTIONS_HOST, this->host.c_str());
	ssh_options_set(this->session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_options_set(this->session, SSH_OPTIONS_PORT, &port);

	/// Connect to server
	rc = ssh_connect(this->session);
	if (rc != SSH_OK) {
		Logger::Log("TTY error connecting to localhost: ", 0);
		Logger::Log(ssh_get_error(this->session));
		ssh_free(this->session);
		ssh_finalize();
		exit(EXIT_FAILURE);
	}

	/// Verify the server's identity
	if (this->verifyKnownHost() < 0) {
		ssh_disconnect(this->session);
		ssh_free(this->session);
		ssh_finalize();
		Logger::Die("TTY host verification failed");
	}

	/// Authenticate ourselves
	if (this->authType.compare("password") == 0) {
		rc = ssh_userauth_password(this->session, NULL, this->password.c_str());
		if (rc != SSH_AUTH_SUCCESS) {
			ssh_disconnect(this->session);
			ssh_free(this->session);
			ssh_finalize();
			Logger::Die("TTY error authenticating with password");
		}
	} else if (this->authType.compare("pubKey") == 0) {
		rc = ssh_userauth_autopubkey(this->session, this->password.c_str());
		if (rc != SSH_AUTH_SUCCESS) {
			ssh_disconnect(this->session);
			ssh_free(this->session);
			ssh_finalize();
			Logger::Die("TTY error authenticating with public key");
		}
	} else {
		ssh_disconnect(this->session);
		ssh_free(this->session);
		ssh_finalize();
		Logger::Die("TTY Invalid authentication type");
	}

	/// Open a channel
	this->channel = ssh_channel_new(this->session);
	if (this->channel == NULL) {
		Logger::Log("TTY couldn't open a channel: ", 0);
		Logger::Log(ssh_get_error(this->session));
		ssh_disconnect(this->session);
		ssh_free(this->session);
		ssh_finalize();
		exit(EXIT_FAILURE);
	}

	/// Set channel blocking/nonblocking
	ssh_channel_set_blocking(this->channel, 1);

	/// Open a channel session
	rc = ssh_channel_open_session(this->channel);
	if (rc != SSH_OK) {
		Logger::Log("TTY couldn't open a channel session: ", 0);
		Logger::Log(ssh_get_error(this->session));
		ssh_channel_close(this->channel);
		ssh_channel_free(this->channel);
		ssh_disconnect(this->session);
		ssh_free(this->session);
		ssh_finalize();
		exit(EXIT_FAILURE);
	}

	/// Request a TTY
	rc = ssh_channel_request_pty_size(this->channel, this->term.c_str(), this->width, this->height);
	if (rc != SSH_OK) {
		Logger::Log("TTY couldn't get a TTY: ", 0);
		Logger::Log(ssh_get_error(this->session));
		ssh_channel_close(this->channel);
		ssh_channel_free(this->channel);
		ssh_disconnect(this->session);
		ssh_free(this->session);
		ssh_finalize();
		exit(EXIT_FAILURE);
	}

	/// Request a shell
	rc = ssh_channel_request_shell(this->channel);
	if (rc != SSH_OK) {
		Logger::Log("TTY couldn't get a shell: ", 0);
		Logger::Log(ssh_get_error(this->session));
		ssh_channel_close(this->channel);
		ssh_channel_free(this->channel);
		ssh_disconnect(this->session);
		ssh_free(this->session);
		ssh_finalize();
		exit(EXIT_FAILURE);
	}

	this->active = 1;
}

WebTTY::TTY::~TTY()
{
	this->active = 0;
	ssh_channel_close(this->channel);
	ssh_channel_free(this->channel);
	ssh_disconnect(this->session);
	ssh_free(this->session);
	ssh_finalize();
}

void WebTTY::TTY::send(std::string input)
{
	if (this->active == 1) {
		ssh_channel_write(this->channel, input.c_str(), input.length());
	}
}

std::string WebTTY::TTY::receive(void)
{
	std::string output = "";
	char buffer[512] = {0};

	if (this->active == 1) {
		while (ssh_channel_poll_timeout(this->channel, 5000, 0) > 0) {
			if (ssh_channel_read_nonblocking(this->channel, buffer, 512, 0) > 0) {
				output += buffer;
			}
			buffer[0] = '\0';
		}
	}

	return output;
}

int WebTTY::TTY::verifyKnownHost()
{
	int state, hlen;
	unsigned char *hash = NULL;
	char *hexa;
	char buf[10];

	state = ssh_is_server_known(this->session);

	hlen = ssh_get_pubkey_hash(this->session, &hash);
	if (hlen < 0) {
		return -1;
	}

	switch (state) {
		case SSH_SERVER_KNOWN_OK:
			break; /* ok */
		case SSH_SERVER_KNOWN_CHANGED:
			//fprintf(stderr, "Host key for server changed: it is now:\n");
			//ssh_print_hexa("Public key hash", hash, hlen);
			//fprintf(stderr, "For security reasons, connection will be stopped\n");
			free(hash);
			return -1;
		case SSH_SERVER_FOUND_OTHER:
			//fprintf(stderr, "The host key for this server was not found but an other"
			//	"type of key exists.\n");
			//fprintf(stderr, "An attacker might change the default server key to"
			//	"confuse your client into thinking the key does not exist\n");
			free(hash);
			return -1;
		case SSH_SERVER_FILE_NOT_FOUND:
			//fprintf(stderr, "Could not find known host file.\n");
			//fprintf(stderr, "If you accept the host key here, the file will be"
			//	"automatically created.\n");
			/* fallback to SSH_SERVER_NOT_KNOWN behavior */
		case SSH_SERVER_NOT_KNOWN:
			/*hexa = ssh_get_hexa(hash, hlen);
			fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
			fprintf(stderr, "Public key hash: %s\n", hexa);
			free(hexa);
			if (fgets(buf, sizeof(buf), stdin) == NULL) {
				free(hash);
				return -1;
			}
			if (strncasecmp(buf, "yes", 3) != 0) {
				free(hash);
				return -1;
			}*/
			if (ssh_write_knownhost(session) < 0) {
				//fprintf(stderr, "Error %s\n", strerror(errno));
				free(hash);
				return -1;
			}
			break;
		case SSH_SERVER_ERROR:
			//fprintf(stderr, "Error %s", ssh_get_error(session));
			free(hash);
			return -1;
	}

	free(hash);
	return 0;
}

std::string WebTTY::TTY::getParam(std::string key)
{
    key.append("=");

    for (std::vector<std::string>::iterator it = this->params.begin(); it != this->params.end(); it++) {
        if(it->substr(0, key.size()) == key) {
            return it->substr(key.size());
        }
    }

    return std::string("");
}
