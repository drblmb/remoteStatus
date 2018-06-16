CXX = g++
CPPFLAGS = 
CXXFLAGS = -Wall
LDFLAGS = -lboost_system -lboost_thread -lpthread 

boostClient: boostClient.cpp
	$(CXX) boostClient.cpp -o $@ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f boostClient
