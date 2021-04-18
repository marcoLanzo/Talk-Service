# Talk-Service

# Abstract:
Creation of a "talk" service via internet managed by a server.
The service keeps track of a list of clients ready to initiate a "conversation ".

Clients (residing, in general, on different machines), after connecting to the
service acquire the list and show it to the user who can connect to
another user available thus starting the conversation.

Two connected clients between them will allow their users to exchange messages as well as one of the
two does not interrupt the conversation, and will be unavailable to other conver-
sations. 

At the end of a conversation the clients will be available again until that do not disconnect from the server.

# Implementation side:
TCP was used as a transport protocol to ensure reliability and security in sending the data flow between clients and server.

The server is built through a multithreaded approach in which each client,
requesting the connection, was entrusted to a private thread that managed the requests.

In the management of conversations between clients, API select was used which allows you to examine multiple I / O channels at the same time and create
hence the I / O multiplexing.

Use of two different types of linked lists:
- the first that keeps track of all users connected to the chat
- the second which allows the server to keep track of all unavailable users.

Use of the SIGINT and SIQUIT signals to allow the user to terminate the conversation with the server.
Use of the SIGALARM signal which will automatically disconnect the user after a predetermined time of inactivity.

# Run Program:

By following the steps below you can run the code on your own
device:
- take the compressed "Talk-Service" file and move it within your file system;
- unzip the folder;
- open two separate terminals and go to the right folder;
- with the first terminal, fill in the "Server.c" file in the "Server" folder;
- with the second terminal, fill in the "Client.c" file in the "Client" folder;
- finally perform both.
