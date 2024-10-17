<p align="center">
   <img src="banner.png" alt="Babs" />
</p>

## Protocol spec
Version 0
1. Client establishes a TCP connection
2. Server accepts and immediately sends a hello message
3. Client sends a login request
4. Server responds with state
5. Server awaits further requests

### Request/response spec
* header (1)
* preamble (2, 3)
* content (4)

1. length: 4 bytes unsigned
2. version: 2 bytes unsigned
3. type: 1 byte unsigned
   * 0 none (to send such a packet is an error)
   * 1 hello
   * 2 state
   * 3 login
   * 4 message
5. content…
   * hello: name (string)
   * state: 1 byte unsigned: 0 ok, 1… error
   * login: uid
   * message: sender's uid, recipient's uid, content (string)

uid: 2 bytes unsigned

all strings are utf-8

## Acknowledgements
Banner background photo: [Phoenicopteridae Phoenicopterus ruber AMERICAN FLAMINGO, NasserHalaweh, Wikimedia Commons](https://commons.wikimedia.org/wiki/File:Phoenicopteridae_Phoenicopterus_ruber_4.1.jpg), CC BY-SA 4.0

Banner font: Mrs Sheppards, Sudtipos, OFL
