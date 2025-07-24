<p align="center">
  <img src="https://github.com/user-attachments/assets/f4a9bd72-25a5-4ca9-a1fe-66faaa0e4710" alt="NSC++" />
</p>

**Networking System C++ (NSCpp)** is a modern C++20 server/client framework designed to simplify network programming in C++.  
It provides robust support for both **IPv4** and **IPv6**, as well as **TCP** and **UDP** communication protocols.  
With ***message framing*** for TCP and the possibility to use **symmetric ciphering**.  

NSCpp is built as a **modular** library, using a flexible **Channel-based architecture** to allow clean separation of code, easy customization, and scalable extension of networking functionality.  

**NB**: This is the second version of the library, which I never officially released before. That’s why you might see it referred to as **NSCpp 2.0** sometimes.

## Table of Contents
- [Security Considerations](#security-considerations)
- [Features](#features)
- [Architecture](#architecture)
- [Communication & Architecture Diagrams](#communication--architecture-diagrams)
- [Documentation](#documentation)
- [Usage Example](#usage-example)
- [Contributing](#contributing)

## Security Considerations
This library relies on my other project, [KISS](https://github.com/Tom-KB/KISS/tree/main), which implements the encryption logic. I’ll explain how to use it later in this document.   
As mentioned in that project, "*overall security depends not only on the backend implementation but also on how you exchange and protect cryptographic keys.*"  
I do not guarantee that this implementation is flawless — feel free to disable it if you prefer. If you find any issues or vulnerabilities, you're more than welcome to report them.   
That said, it's up to you to ensure proper key management and integration in your specific use case.

## Features
This project is a C++ overlay of my [NSC](https://github.com/Tom-KB/NSC/tree/main) project — with additional features!  
Here’s a list of what NSC++ offers:
 - **IPv4** and **IPv6** support
 - **TCP** and **UDP** communication protocols (both wrapped inside the **Server** and **Client**)
 - ***Message framing*** for TCP
 - **Domain Name resolution**
 - An **event** system
 - **Windows** and **Linux** support
 - A **Channel-based architecture** 
 - A **separator system** to structure your message
 - A **serializer** to easily manipulate messages using the separator format
 - A **group system** to exchange information across multiple channels
 - **Key exchange** and **symmetric encryption**, powered by the libsodium backend of [KISS](https://github.com/Tom-KB/KISS/tree/main)
 - 3 **basic channels** :
   + *Debugging*
   + *Channels discovery* (informs clients which channels are plugged)
   + *Logging*
 - A **client wrapper** with a **convenient callback system** to easily handle and process incoming data

## Architecture
Here is the class diagram I designed and used to build this project :
<img width="1780" height="1295" alt="image" src="https://github.com/user-attachments/assets/9d071d71-64fc-4a37-8288-5d528017b014" />
**NB**: There are some differences between this diagram and the final implementation.  
During development, I made a few *implementation choices* that aren’t reflected in the diagram.

## Documentation
I’ve also generated a full [documentation](https://tom-kb.github.io/NSCpp/annotated.html) using Doxygen.  
Below is a brief description of each class, along with a link to its detailed reference in the documentation
  - [ServerPP](https://tom-kb.github.io/NSCpp/class_server_p_p.html) : Used to instantiate the server and plug in your custom channels
  - [ClientPP](https://tom-kb.github.io/NSCpp/class_client_p_p.html) : Used to instantiate the client, register event callbacks, and handle communication with the server
  - [Channel](https://tom-kb.github.io/NSCpp/class_channel.html) : Abstract base class to derive when creating custom channels
    + [DebugChannel](https://tom-kb.github.io/NSCpp/class_debug_channel.html) : A debug channel that logs messages to a given output stream and can echo them back to the sender
    + [HelperChannel](https://tom-kb.github.io/NSCpp/class_helper_channel.html) : A channel discovery tool that, when triggered, responds to the sender with a list of all currently plugged channels
    + [LogChannel](https://tom-kb.github.io/NSCpp/class_log_channel.html) : A logging channel that can either actively log all server messages or be triggered via a group to log specific input
  - [Serializer](https://tom-kb.github.io/NSCpp/class_serializer.html) : A singleton class for manipulating strings structured using the separator system
  - [GroupManager](https://tom-kb.github.io/NSCpp/class_group_manager.html) : A singleton class to manage channels within a group and handle communication across them

## Communication & Architecture Diagrams
**NB** : For these diagrams, I used the "/" as separator.

### Key Exchange & Symmetric ciphering
<img width="1609" height="745" alt="1" src="https://github.com/user-attachments/assets/0212c373-5d3d-4446-8f87-d20e10cee071" />

### Channel system
<img width="1135" height="810" alt="2" src="https://github.com/user-attachments/assets/6fb78927-0838-4758-a7ec-6face87d061a" />

Here is a simple example showing how to instantiate channels and plug them into the server:
```cpp
shared_ptr<Channel> debugChannel = make_unique<DebugChannel>("debugChannel", cout);
shared_ptr<LogChannel> logChannel = make_unique<LogChannel>("logChannel", "log-");
shared_ptr<HelperChannel> helperChannel = make_unique<HelperChannel>("helperChannel");
server->plug(debugChannel, ChannelType::GLOBAL | ChannelType::METADATA);
server->plug(helperChannel, ChannelType::DATA);
server->plug(logChannel, ChannelType::SLEEPING);
```

## Usage example
In the [examples](examples/) folder, you’ll find the same example as in [NSC](https://github.com/Tom-KB/NSC)—C^LI adapted for NSC++.  
Currently, it’s developed specifically for Windows, as I’m working with Visual Studio 2022, but I may add a Makefile for C^LI++ on Linux in the future.

You’ll also find additional examples showcasing specific use cases of NSC++ that I consider important.

### Server's creation with specific keys
```cpp
unique_ptr<ServerPP> server = make_unique<ServerPP>("127.0.0.1", 25565, "/", IP_Type::IPv4, true, "public.key", "private.key");
server->start();
// ...
server->stop();
```
If you don't specify the public and private keys, a new key pair will be automatically generated and saved using default filenames.
You can then share the **public key** with your clients and reuse the generated keys for your server.

### Client's creation
```cpp
unique_ptr<ClientPP> client = make_unique<ClientPP>("127.0.0.1", 25565, "/", IP_Type::IPv4, true);
client->setServerPKPath("serverPK.key");
client->setKeysPaths("public.key", "private.key"); // Optional
client->start();
// ...
client->stop();
```
The same applies to clients: if you want to reuse the same public/private key pair, you can specify them manually. Otherwise, a new key pair will be generated and saved each time the client runs.

### Client's message
```cpp
std::string message = "...";
client->send(message, ConnType::TCP);
// or
client->send(message, ConnType::UDP);
```
This is the client-sided behaviour.

### Client's callback
```cpp
client->addDataCallback("", generalCallback); // General callback (no key)
client->addDataCallback("group1", anotherCallback); // Callback triggered by the "group1" key
client->addDisconnectionCallback(disconnectionCallback); // Disconnection callback (can hold multiple callbacks)
```
**For now**, only one callback can be registered per key. I believe that if you need to notify multiple handlers, you can build that logic within a single callback.
However, if I later find that supporting multiple callbacks per key is truly essential, I’ll consider adding it as a feature.

### Channels
```cpp
// Channel example
void ExampleChannel::run(stop_token st) {
    stringstream ss;
    GroupManager& groupManager = GroupManager::getInstance();
    Serializer& serializer = Serializer::getInstance();
    while (!st.stop_requested()) {
        if (available()) {
            ss.str("");
            ClientData clientData = pull();

            if (clientData.type == Connection) {
                // Do something
            }
            else if (clientData.type == Disconnection) {
                // Do something
            }
            else if (clientData.type == DataReceived) {
                // The data is split using the separator defined during the server’s creation
                for (int i = 0; i < clientData.data.size(); ++i) {
                    ss << clientData.data[i] << endl;
                }
                if (logging) groupManager.send(logKey, "[EXAMPLE] Received messsage by " + clientData.client->getID() + " : " + ss.str() + "\n");
                clientData.client->send(serializer.get(key));

                if (echo) {
                    switch (clientData.connType) {
                    case ConnType::TCP:
                        clientData.client->send(ss.str(), ConnType::TCP);
                        if (logging) groupManager.send(logKey, "[EXAMPLE] [TCP] ECHO : " + ss.str());
                        break;
                    case ConnType::UDP:
                        sendUDP(&clientData.socket, &clientData.addr, clientData.ipType, ss.str());
                        if (logging) groupManager.send(logKey, "[EXAMPLE] [UDP] ECHO : " + ss.str());
                        break;
                    }
                }
            }
        }
    }
}
```
This example demonstrates several common use cases for channel usage.
Of course, if a channel isn't designed to handle **METADATA**, you can safely remove the `if` statements related to connection and disconnection events (the same goes for non **DATA** nor **GLOBAL** channels).

You’ll also see how the group system works in practice: for instance, when a *LoggingChannel* is created, it’s automatically added to a group **based on its key**. Sending data to that group will result in it being logged.

This example also highlights an important point: to respond correctly to a client based on the communication protocol, you may need to adapt your usage patterns accordingly.  
If you need to associate a **TCP socket** with its corresponding **UDP address**, you’ll need to implement that logic yourself.
(I might add a dedicated channel for this in the future, but it *won’t* be included as part of the core set.)

## Contributing
Contributions are welcome.  
Feel free to submit bug fixes or suggest new features.  
However, please **don’t propose new channels in this repository**.  
I’ll be creating a separate repository specifically for the channels I develop, along with proper documentation for each.
