Protocol spec
Version 0
1. Client establishes a TCP connection
2. Server accepts and immediately sends a hello message
3. Client sends a login request
4. Server responds with state
5. Server awaits further requests

Request/response spec
header (1)
preamble (2, 3)
content (4)

1. length: 4 bytes unsigned
2. version: 2 bytes unsigned
3. type: 1 byte unsigned
   0 hello
   1 state
   2 login
   3 message
4. content…
   hello: name (string)
   state: 1 byte unsigned: 0 ok, 1… error
   login: uid
   message: sender's uid, recipient's uid, content (string)

uid: 2 bytes unsigned
all strings are utf-8
