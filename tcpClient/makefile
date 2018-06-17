CXX = g++
CPPFLAGS = 
CXXFLAGS = -Wall
LDFLAGS =

tcpClient: TcpClientSide.cpp TcpClient.cpp TcpSocket.cpp
	$(CXX) TcpClientSide.cpp TcpClient.cpp TcpSocket.cpp -o $@ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f tcpClient
