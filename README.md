This project has been created as part of the 42 curriculum by < mohel-kh > < moelgham >.

ft_irc
Description
This project is about creating our own Internet Relay Chat (IRC) server. The goal was to build a fully functional server in C++98 that can handle multiple clients simultaneously without blocking.

It was a fascinating dive into network programming. We had to implement the core IRC protocol to allow users to communicate in real-time. The server supports authentication, joining channels, private messaging, and a set of operator commands to manage the chat rooms. Essentially, it mimics the behavior of real-world IRC servers, allowing standard IRC clients (like HexChat or irssi) to connect and interact with it.

Key Features
The server implements the essential features required by the IRC protocol:

Authentication: Users must provide a password, nickname, and username to connect.
Channels: Users can create, join, and leave channels.
Messaging: Support for both private messages (between users) and channel broadcasts.
Operator Commands:
KICK: Eject a user from a channel.
INVITE: Invite a user to a channel.
TOPIC: Change or view the channel topic.
MODE: Manage channel modes (invite-only, topic protection, key/password, operator privileges, user limit).
Instructions
CompilationTo compile the server, simply run:

make
This will generate the ircserv executable. The code compiles with c++ using the flags -Wall -Wextra -Werror and adheres to the C++98 standard.

Running the Server
Launch the server by providing a port number and a connection password:

bash

./ircserv <port> <password>
<port>: The port number (e.g., 6667).
<password>: The password clients need to connect.
Connecting
You can connect using any standard IRC client. For a quick test using netcat:

bash

nc -C 127.0.0.1 6667
PASS password
NICK nickname
USER username
Testing
We ensured the server handles partial data sent by clients (like sending a command in multiple packets). The server uses poll() to manage file descriptors efficiently, ensuring non-blocking I/O operations throughout the execution.

Resources
Here are some of the resources that were incredibly helpful during development:

RFC 1459 - Internet Relay Chat Protocol: The holy grail for understanding IRC standards.
Modern IRC Client Protocol: A cleaner, more readable version of the protocol documentation.
AI Usage
In this project, we strictly limited our use of AI tools to a specific task: identifying the numeric reply codes.

Since the IRC protocol relies on specific numbers for different responses (like 001 for welcome messages or 461 for parameter errors), we used AI to quickly look up these standard codes to ensure our server's replies were compliant with the protocol, saving us time that would have been spent manually searching through the RFC documentation.