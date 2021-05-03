/**************************************************************************
Copyright:
File Name: server.cpp
Author:    Qiang Fan
Date:2021-04-29
Modified: 2021-05-02
Version: V2.0
Description: This file to realize the functions 
of a server encrpted by ssl.				
***************************************************************************/

#include "server.h"
vector<SSL *> ssl_pool;
vector<string> user_pool;
map<string, SSL *> user_ssl_pool;
pthread_t tid[20];
int ti = 0;

// @Description: create the socket at the server and listen to the client requests
// @param: port#
// @return: socket
// @birth:
int OpenListener(int port)
{
    int sock;
    struct sockaddr_in addr; /*creating the sockets*/
    sock = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));  /*free output the garbage space in memory*/
    addr.sin_family = AF_INET;   /*getting ip address form machine */
    addr.sin_port = htons(port); /* converting host bit to n/w bit */
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) /* assiging the ip address and port*/
    {
        perror("can't bind port"); /* reporting error using errno.h library */
        abort();                   /*if error will be there then abort the process */
    }
    if (listen(sock, 10) != 0) /*for listening to max of 10 clients in the queue*/
    {
        perror("Can't configure listening port"); /* reporting error using errno.h library */
        abort();                                  /*if erroor will be there then abort the process */
    }
    return sock;
}

// @Description: creating and setting up ssl context structure
// @param: void
// @return: ssl context structure
// @birth:
SSL_CTX *InitServerCtx(void)
{
    SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();                   /* load & register all cryptos, etc. */
    SSL_load_error_strings();                       /* load all error messages */
    method = (SSL_METHOD *)TLSv1_2_server_method(); /* create new server-method instance */
    ctx = SSL_CTX_new(method);                      /* create new context from method */
    if (ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

// @Description: to load a certificate into an SSL_CTX structure
// @param: ctx: ssl context structure; cert_file: certificate file; key_file: key file
// @return: void
// @birth:
void LoadCert(SSL_CTX *ctx, char *cert_file, char *key_file)
{
    /* set the local certificate from cert_file */
    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from key_file (may be the same as cert_file) */
    if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* verify private key */
    if (!SSL_CTX_check_private_key(ctx))
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

// @Description: Show the ceritficates to client and match them;
// @param: ssl: ssl connection
// @return: void
// @birth:
void PrintCert(SSL *ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if (cert != NULL)
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Server: %s\n", line); /*server certifcates*/
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("client: %s\n", line); /*client certificates*/
        free(line);
        X509_free(cert);
    }
    else
        printf("No certificates.\n");
}

// @Description: send messages to all clients in a group mode;
// @param: msg--> data stream; source_user--> username of the source client
// @return: void
// @birth:
void SendToMembers(char *msg, string source_user)
{
    if (user_ssl_pool.size() <= 1)
    {
        char err_msg[] = "Warn: No Group member!\n";
        SSL_write(user_ssl_pool[source_user], err_msg, strlen(err_msg));
        return;
    }
    map<string, SSL *>::iterator iter;
    for (iter = user_ssl_pool.begin(); iter != user_ssl_pool.end(); iter++)
    {
        if (iter->first == source_user)
            continue;
        string temp = msg;
        string sum_msg = "client " + source_user + "->Group:" + msg;
        char input[1024];
        strncpy(input, sum_msg.c_str(), sum_msg.length() + 1);
        SSL *temp_ssl = iter->second;
        SSL_write(temp_ssl, input, strlen(input));
    }
}
// @Description: send messages to all clients in a group mode;
// @param: msg--> data stream; ssl--> ssl tunnel of source user; source_user--> username of transmitter
//         target_user--> username of receiver
// @return: void
// @birth:
void SendToOne(char *msg, SSL *ssl, string source_user, string target_user)
{
    char *err_msg = (char *)malloc(40);
    if (user_ssl_pool.count(target_user) == 0)
    {

        strcpy(err_msg, "Warn: No target user!\n");
        SSL_write(user_ssl_pool[source_user], err_msg, strlen(err_msg));
        return;
    }
    char input[1024];
    strcpy(input, "-Private:");
    strcat(input, msg);
    SSL_write(user_ssl_pool[target_user], input, strlen(input));
    free(err_msg);
}

// @Description: create thread for each client
// @param: ssl: ssl connection
// @return: void
// @birth:
void SendReceiveData(SSL *ssl)
{
    if (SSL_accept(ssl) == FAIL) /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    pthread_create(&tid[ti++], NULL, recv_thread, ssl);
}

// @Description: the thread handler for the server to broadcast msg;
// @param: args : NULL
// @return: void
// @birth:
void *send_thread_broad(void *args)
{
    char *input = (char *)malloc(BUFFER);
    cout << "After clients join with usernames, you can check the list by typing 'List' here!" << endl;
    while (1)
    {
        printf("\nMsg to all clients:");
        fgets(input, BUFFER, stdin); /* get request and reply to client*/
        if (strncmp(input, "List", 4) == 0)
        {
            map<string, SSL *>::iterator iter;
            cout << "Client List: " << endl;
            for (iter = user_ssl_pool.begin(); iter != user_ssl_pool.end(); iter++)
            {
                cout << " " << iter->first << " ";
            }
            continue;
        }
        for (int k = 0; k < (int)ssl_pool.size(); k++)
        {
            SSL_write(ssl_pool[k], input, strlen(input));
        }
    }
    free(input);
    return NULL;
}

// @Description: the thread handler for forwarding each source client's data;
// @param: args : args--> ssl pointer of the source client
// @return: void
// @birth:
void *recv_thread(void *args)
{
    char buf[1024];
    char log_msg[100];
    char source_user[50];
    char target_user[50];
    char cmdone[] = "O";
    char cmde[] = "E";
    char cmdg[] = "G";

    SSL *ssl = (SSL *)args;
    strcpy(log_msg, "Please register a username for this client f:\n");
    SSL_write(ssl, log_msg, strlen(log_msg));
    while (1)     /* register and check username for a joining client*/
    {
        SSL_read(ssl, buf, sizeof(buf));

        string temp = buf;
        strcpy(source_user, buf);
        if (user_ssl_pool.count(temp) > 0)
        { /* check if the username exists or not*/
            strcpy(log_msg, "The username exists, please input a new one:");
            SSL_write(ssl, log_msg, strlen(log_msg));
            continue;
        }

        user_ssl_pool[temp] = ssl;
        user_pool.push_back(temp);
        cout << temp << " joins the chat!" << endl;
        break;
    }

    while (1)
    {
        /*Select the command to select P2P mode or group mode*/
        char *cmd = (char *)malloc(50);
        strcpy(log_msg, "Select chat mode: One on One:O; group:G; to exit current mode:E");
        SSL_write(ssl, log_msg, strlen(log_msg));
        SSL_read(ssl, cmd, sizeof(cmd)); /* get request and read message from server*/

        /*Start the P2P mode*/
        if (strncmp(cmd, cmdone, 1) == 0)
        { //	if (strcmp(buf,"cmd one")==0){
            strcpy(log_msg, "type target username:");
            SSL_write(ssl, log_msg, strlen(log_msg));
            SSL_read(ssl, buf, sizeof(buf)); /* get request and read message from server*/
            strcpy(target_user, buf);

            while (1)
            {
                memset(buf, 0, 1024 * sizeof(char));
                SSL_read(ssl, buf, 1024 * sizeof(char)); /* get request and read message from server*/
                string source = source_user;
                string target = target_user;
                if (strncmp(buf, cmde, 1) == 0)
                    break;
                SendToOne(buf, ssl, source, target);
            }
        }
        
        /*Start the group mode*/
        else if (strncmp(cmd, cmdg, 1) == 0)
        {
            cout << "cmd g" << endl;
            strcpy(log_msg, "You are in group mode:");
            SSL_write(ssl, log_msg, strlen(log_msg));
            while (1)
            {
                memset(buf, 0, 1024 * sizeof(char));
                SSL_read(ssl, buf, 1024 * sizeof(char));
                string source = source_user;
                if (strncmp(buf, cmde, 1) == 0)
                    break;
                SendToMembers(buf, source);
            }
        }
        free(cmd);
    }
    free(buf);
    return NULL;
}


// @Description: After each client set up a ssl channel, 
//              create a thread for each of them to process the sent data and recved data
// @param: listen_sock; client_addr; len-->length of addr; ctx;
// @return: void
// @birth:
void CheckMultiSock(int listen_sock, struct sockaddr_in client_addr, socklen_t len, SSL_CTX *ctx)
{
    pthread_create(&tid[ti++], NULL, send_thread_broad, NULL);
    while (1)
    {
        int connected_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &len);             /* get the current connected socket towards a client */
        printf("Connection: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)); /*printing connected client information*/
        SSL *ssl;
        ssl = SSL_new(ctx);              /* create SSL connection based on certificate of the server */
        SSL_set_fd(ssl, connected_sock); /* attach the ssl connection to the client socket */
        ssl_pool.push_back(ssl);
        SendReceiveData(ssl); /* Send or receive messges */
    }
}

// @Description: The main function of the server
// @param: count; *strings[]: port # of server
// @return: int
// @birth:
int main(int count, char *strings[])
{
    SSL_CTX *ctx;
    int listen_sock;
    char *portnum;
    if (count != 2)
    {
        printf("Usage: %s \n", strings[0]); /*send the usage guide if syntax of setting port is different*/
        exit(0);
    }
    SSL_library_init(); /*load encryption and hash algo's in ssl*/
    portnum = strings[1];
    ctx = InitServerCtx(); /* initialize ssl context */
    char cert[] = "certificate.pem", key[] = "certificate.pem";
    LoadCert(ctx, cert, key);                  /* load certicate of the server to the ssl context */
    listen_sock = OpenListener(atoi(portnum)); /* create listen socket for server */
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    //SSL *ssl;
    listen(listen_sock, 5); /*setting 5 clients at a time to queue*/
    CheckMultiSock(listen_sock, client_addr, len, ctx);
    close(listen_sock); /* close server's listen socket */
    SSL_CTX_free(ctx);  /* release ssl context */
    return 0;
}
