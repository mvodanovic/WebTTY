WebTTY
A web based TTY application (in making).

The idea is to create a fully functional PuTTY-like web application which could be installed & run on any web server. I don't know yet if I'll run into a dead end, hoping it won't come to that. And if it does, at least it will have been a fun project which taught me something new.

I found some similar attempts to implement something like this, but none of them are good enough. I want this app to be fully functional like any other terminal emulator, meaning i can run applications like "vim" or "htop" in it with no problems. The goal is to be compatible with modern xterm-256color terminal emulator implementations.

I've divided up the work in these approximate phases:
1) Implement a SSH daemon (using libssh) [C/C++]
2) Implement an interface to the daemon which will communicate with it [C/C++]
3) Implement web server scripts which communicate with the daemon interface and connect to the localhost [PHP]
4) Implement a web UI which will communicate with the web server [HTML5/CSS3/JS/jQuery=WEB]
5) Add the possibility to store & use multiple configurations for connecting with different servers [PHP,WEB]
6) Addd the possibility to open multiple TTYs in a single session (I.E. in multiple tabs) [PHP,WEB]

The basic idea so far is this... When a new connection is made, the daemon forks a new process which will handle it. If the client doesn't respond for some time, the daemon kills the corresponding process. On the client side there's a rich-text read-only display for the terminal and a masked input with special key bindings for data input. Every keystroke is sent to the server and the display is updated only when a response is received from the server. The web part will also have a comet model implemented which will continuously ping the serer which will in turn ping the daemon and keep the connection alive. This model will be responsible for fetching output data to the user.

That's about it. Currently I'm in the early stages of phase 1. I'll update this readme as the development progresses. Any feedback or assistance is apprecieted.

The project is copyrighted under the GNU GPL:
http://www.gnu.org/licenses/gpl.html
