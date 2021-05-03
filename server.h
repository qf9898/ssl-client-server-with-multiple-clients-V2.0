#ifndef __SERVER_H_H1__
#define __SERVER_H_H1__
#include <unistd.h> /*FOR USING FORK for at a time send and receive messages*/ 

#include <errno.h>   /*USING THE ERROR LIBRARY FOR FINDING ERRORS*/
#include <malloc.h>  /*FOR MEMORY ALLOCATION */
#include <iostream>
#include <string>  /*using fgets funtions for geting input from user*/

#include <arpa/inet.h>  /*for using ascii to network bit*/ 

#include <sys/socket.h>  /*for creating sockets*/

#include <sys/types.h>  /*for using sockets*/

#include <netinet/in.h>        /* network to asii bit */

#include <resolv.h>  /*server to find out the runner's IP address*/ 

#include "openssl/ssl.h" /*using openssl function's and certificates and configuring them*/

#include "openssl/err.h" /* helps in finding out openssl errors*/

#include <stdio.h>   /*standard i/o*/

#include <pthread.h>

#include <vector>

#include <map>
using namespace std;
using std::vector;
using std::string;
using std::map;

#define FAIL    -1  /*for error output == -1 */

#define BUFFER 1024  /*buffer for reading messages*/

int OpenListener(int port);
SSL_CTX* InitServerCtx(void);		 /*creating and setting up ssl context structure*/
void LoadCert(SSL_CTX* ctx, char* cert_file, char* key_file);   /* to load a certificate into an SSL_CTX structure*/
void PrintCert(SSL* ssl);	/*show the ceritficates to client and match them*/
void SendReceiveData(SSL* ssl);		/* Serve the connection -- threadable */
int main(int count, char *strings[]);   /* getting port as a argument*/

void *send_thread_broad(void *args);
void *recv_thread(void* args);
void CheckMultiSock(int listen_sock, struct sockaddr_in client_addr, socklen_t len, SSL_CTX *ctx );
void SendToMembers(char *msg, string source_user);
void SendToOne(char *msg, SSL *ssl, string source_user, string target_user);

#endif
