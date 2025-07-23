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
- [Usage Example](#usage-example)
- [Architecture](#architecture)
- [Communication & Architecture Diagrams](communication-&-architecture-diagrams)
- [Documentation](#documentation)
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
 - **TCP** and **UDP** communications protocols (both wrapped inside the **Server** and **Client**)
 - ***Message framing*** for TCP
 - **Domain Name resolution**
 - An **event** system
 - **Windows** and **Linux** support
 - A **Channel-based architecture** 
 - A **separator system** to structure your message
 - A **serializer** to easily manipulate messages using the separator format.
 - A **group system** to exchange information across multiple channels.
 - **Key exchange** and **symmetric encryption**, powered by the libsodium backend of [KISS](https://github.com/Tom-KB/KISS/tree/main)
 - 3 **basic channels** :
   + *Debugging*
   + *Channels discovery* (informs clients which channels are plugged)
   + *Logging*
 - A **client wrapper** with a **convenient callback system** to easily handle and process incoming data.

## Usage example

## Architecture
Here is the class diagram I designed and used to build this project :
<img width="1780" height="1295" alt="image" src="https://github.com/user-attachments/assets/9d071d71-64fc-4a37-8288-5d528017b014" />
**NB**: There are some differences between this diagram and the final implementation.  
During development, I made a few *implementation choices* that aren’t reflected in the diagram.

## Communication & Architecture Diagrams
TODO : Key exchange's diagram, channel system diagram, group system diagram, serialization diagram, other if i have the idea.

## Documentation
I’ve also generated a full [documentation](https://tom-kb.github.io/NSCpp/annotated.html) using Doxygen.
Below is a brief description of each class, along with a link to its detailed reference in the documentation
  - [ServerPP](https://tom-kb.github.io/NSCpp/class_server_p_p.html) : ...
  - [ClientPP](https://tom-kb.github.io/NSCpp/class_client_p_p.html) : ...
  - [Channel](https://tom-kb.github.io/NSCpp/class_channel.html) : ...
    + [DebugChannel](https://tom-kb.github.io/NSCpp/class_debug_channel.html) : ...
    + [HelperChannel](https://tom-kb.github.io/NSCpp/class_helper_channel.html) : ...
    + [LogChannel](https://tom-kb.github.io/NSCpp/class_log_channel.html) : ...
  - [Serializer](https://tom-kb.github.io/NSCpp/class_serializer.html) : ...
  - [GroupManager](https://tom-kb.github.io/NSCpp/class_group_manager.html) : ...

## Contributing
Contributions are welcome.  
Feel free to submit bug fixes or suggest new features.  
However, please **don’t propose new channels in this repository**.  
I’ll be creating a separate repository specifically for the channels I develop, along with proper documentation for each.
