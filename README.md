Project: ssl secure tunnel for client-server V2.0

About:
This is an extension based on previous ssl based client-server chat V1.0.  
The openssl library is employed to generate the certificate and realize the ssl secure tunnel.
The project is developed based on Linux OS (Ubuntu)

### New functionalities as compared to V1.0:
   1. Multiple clients connect to the server through multi-threads.
   2. Each client can register at the server and chat with other clients
   3. Each client can switch between Point2Point mode and group mode.
   4. The server maintains username list and ssl tunnels via HashMap and can broadcast messages to all clients any time.

### Workflow of the software:
At the side of client：
   1. input the unique username to register at the server (for an existing one, retry it based on the software's reminder)
   2. input command to slect Point2Point mode or group mode ('O': Point2Point(i.e., one on one); 'G': group mode; 
   'E': exit current mode and repeat step 2 again.)
   
   3. in Point2Point mode, the client needs to input the username of target client, and then sends message.
   4. input 'E' anytime to exit the current mode
   5. input 'G' to send message to client group.

At the side of server:
   1) Server can maintain and print the user list anytime via command 'List';
   2) When the server has some messages, it can send messages to all clients simultaneously. 
 
### Build:
The code is compiled on Ubuntu OS.
As the project utilizes openssl lib, the lib needs to be installed in advance.

### install openssl library 
$ sudo apt-get install libssl-dev
(if doesn't work, plase also try $ sudo apt install libssl-dev)
### build the codes
1. save the package to your favorable directory (project directory) 
2. Open a terminal and reach project directory: 
   $ cd project-directory

   Creating a certificate file with OpenSSL lib: (optional, the certificate.pem is already attached in the package) 
   $ openssl req -x509 -nodes -days 365 -newkey rsa:1024 -keyout certificate.pem -out certificate.pem

3. Compile the ssl-server and ssl-user (Different from V1.0, g++ is used as compiler due to hashmap and vector in V2.0):

   $ make all

4. Open four Terminals A,B,C,D and reach the project directory ($ cd project-directory)
   For termal A (ssl-server):
   $ ./ssl-server port#       (e.g. $ ./ssl-server 4080)

   For Terminal B C D (ssl-client):
   $ ./ssl-client 127.0.0.1 port#   (e.g., $ ./ssl-client 127.0.0.1 4080)
   Type the username for each termianl according to the hint: (e.g., user1 user2 user3)
   
   Take Termianl B with username "user1" for example:
   1) input command 'O' to start P2P mode
   2) input "user2" (i.e., ) as chat with it.
   3) send messages to user2
   4) stop P2P mode anytime via command 'E'  
   5) then, input command 'G' to start Group mode
   6) stop the mode anytime via command 'E' if you wanna switch the modes anytime.

5. At last, to release the project, executable files (i.e., ssl-server and ssl-user) can be removed to save package size:
   $ make clean 

