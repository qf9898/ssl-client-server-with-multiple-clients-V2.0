all:server.cpp client.cpp
	g++ -Wall -o ssl-server server.cpp -pthread -L/usr/lib -lssl -lcrypto
	g++ -Wall -o ssl-client client.cpp -pthread -L/usr/lib -lssl -lcrypto
clean:ssl-server ssl-client
	rm ssl-server ssl-client
