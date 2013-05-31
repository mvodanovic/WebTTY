#ifndef WEBTTY_TTY__H
#define WEBTTY_TTY__H

#include <string>
#include <libssh/libssh.h>

namespace WebTTY
{
    class TTY
    {
        public:
            TTY(std::string);
            ~TTY();
            void send(std::string);
            std::string receive(void);

        protected:
            ssh_session session;
            ssh_channel channel;
            int verbosity;
            std::string host;
            int port;
            std::string password;
            std::string term;
            int width;
            int height;
            int active;

            int verifyKnownHost();
    };
}

#endif
