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
TCP was used as a transport protocol to ensure reliability and security in sending the data flow between clients and servers.
