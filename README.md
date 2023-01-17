# Authors
Arik Tatievski

Roi Meshulam

# What is this project?

This project is divided into 4 mini-projects:

Part A - A synchronized server supporting clients pushing data through active-object function that recives a string and retunrs the string+1 ("abc" turns to "bcd","123" turn to "456" etc..)

Part B - Guard design pattern implication

Part C - Singleton design pattern implication

Part D - A server that is a basic live chat app supporting many clients through 2 active threads (A main thread to accept clients, A handling thread to "Receive/Send" client messages)

# The way we made our project

(We will only specify Part A&D, you can find information about Part B&C everywhere online):

*Part A:*

The server runs an infinite loop and can listen to 10 users at the same time.

While booting the server we are already activating a pipeline with 3 active-object queues and functions.

When the server gets a connection he runs a thread to handle the sapsific clients request - The thread simply pushes the data inside the pipeline.

Client can send any data he wants as long as it is a string, if the client passes the string "EXIT", the program stops.

The server suppots synchronized through mutex.

*Part D:*

The server runs an infinite loop and can listen to a dynamic number of clients (starting with 5)

The server saves his self-defined file-descriptor used for listening to new clients.

When a client is connected, the server saves his file-descriptor in poll-fds array and the function he is suppose to be doing (Listen/Write to chat), so whenever a client chooses to "perform an action" the server will know which of his client performed and which act he chooses to do.

If we wish to be practical - when ever a client sends a messege the server "publishes" it to the rest of the clients, if the clients messege is "EXIT" he disconnects from the chat 

# How to use our project:

Write in a bash environment "make partA","make partB","make partC","make partD" according to which program you wish to use.

*Part A:* - Active server - "./Server" #### Active client - "./Client"

*Part B:* - "./guard"

*Part C:* - "./singleton"

*Part D:* - Active server - "./pollServer" #### Active client - "./pollClient"


# Extras
You can also write in a bash environment "make test" and run the ./Test

*Test logic:*
We will be testing our active-object functions by creating a custom-made server. (We already check the connectivity&synchronization in task 3-4-5)

We will be pushing "problamatic" data into our active-object and just simply check if our active-object managed to handle it.

# Hope you find good usuage of this project!
