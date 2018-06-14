!testDaemon
The testDaemon is a daemon that will install itself listening on port 
5001.  There is currently no clean way to exit, so it must be killed by
finding the process (ps -ef|grep testDaemon), and killing it by process id.
!!Building testDaemon
* cd testDaemon
* make
This will produce the testDaemon executable which can be run ./testDaemon.
You can verify it is listening on port 5001 by netstat -ant|grep 5001

!testClient
The testClient is a tcp client that will take two parameters:
```./testClient <server> <port>```
where <server> is either the IP address or domain name of the server,
and <port> is the listening port, in this case 5001.
!!Building testClient
* cd testClient
* make

Run the client like `./testClient localhost 5001`.  
Just as with the testDaemon, no clean way to exit, so use <ctrl>+c


