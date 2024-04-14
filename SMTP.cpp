#include <stdlib.h>
#include <sys/socket.h>
#include <memory.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <regex>

char HELO_COMMAND[] = "EHLO test\n";
char MAIL_FROM[] = "MAIL FROM:<%s@%s>\t\n";
char RCPT_TO[] = "RCPT TO:<%s>\r\n";
char DATA[] = "DATA\r\n";
char SUBJECT[] = "Subject: %s\n";
char FROM[] = "From: %s@%s\n";
char TO[] = "To: %s\n";
char MESSAGE[] = "%s\r\n";
char AUTH_LOGIN[] = "AUTH LOGIN\r\n";
char LOGIN[] = "ZGFuaWlsLmdyaXNoYUByYW1ibGVyLnJ1\r\n";
char PASSWORD[] = "RGFuaWxrYTEy\r\n";
char QUIT[] = "quit\r\n";

void runSmtpCommand(char *message, int s, int wait) {
    char response[1024];
    memset(response, 0, sizeof(response));
    send(s, message, strlen(message), 0);
    std::cout << "Sending: " << message << std::endl;
    std::cout << "Waiting..." << std::endl;
    recv(s, response, sizeof(response), 0);
    std::cout << "Got response: " << response << std::endl << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Expected one argument!" << std::endl;
        exit(-1);
    }
    std::regex e("[a-zA-Z0-9._-]+@[a-zA-Z0-9._-]+\\.[a-zA-Z0-9_-]+");
    if (!std::regex_match(argv[1], e)) {
        std::cout << "Incorrect mail address format!" << std::endl;
        exit(-1);
    }
    int error;
    hostent *d_addr;
    struct sockaddr_in addr;
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    d_addr = gethostbyname("smtp.rambler.ru");
    if (!d_addr) {
        std::cout << "Host getting is failed!" << std::endl;
        exit(-1);
    }
    std::cout << d_addr << std::endl;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = *((unsigned long *) d_addr->h_addr);
    addr.sin_port = htons(2525); // порт
    if (connect(s, (sockaddr * ) & addr, sizeof(addr)) == -1) {
        std::cout << "Connection creating error!" << std::endl;
        exit(-1);
    } else {
        std::cout << "OK" << std::endl;
    }
    char response[1024];
    memset(response, 0, sizeof(response));
    recv(s, response, sizeof(response), 0);
    char login[LOGIN_NAME_MAX];
    char hostname[HOST_NAME_MAX];
    strcpy(login, "daniil.grisha");
    strcpy(hostname, "rambler.ru");
    // Посылка EHLO
    runSmtpCommand(HELO_COMMAND, s, 0);
    runSmtpCommand(AUTH_LOGIN, s, 0);
    runSmtpCommand(LOGIN, s, 0);
    runSmtpCommand(PASSWORD, s, 0);
    // Посылка MAIL FROM
    char *buf = new char[BUFSIZ];
    sprintf(buf, MAIL_FROM, login, hostname);
    runSmtpCommand(buf, s, 0);
    // Посылка RCPT TO
    buf = new char[BUFSIZ];
    sprintf(buf, RCPT_TO, argv[1]);
    runSmtpCommand(buf, s, 0);
    // Посылка DATA
    runSmtpCommand(DATA, s, 0);
    // Подготовка заголовка
    char finalMessage[BUFSIZ] = "";
    char tempFrom[100];
    sprintf(tempFrom, FROM, login, hostname);
    strcat(finalMessage, tempFrom);
    char tempTo[100];
    sprintf(tempTo, TO, argv[1]);
    strcat(finalMessage, tempTo);
    std::cout << "Subject: ";
    char subject[1024];
    int i = 0;
    char n, t;
    while ((n = getchar()) != '\n' and n != EOF) {
        subject[i++] = n;
    }
    char tempSubject[BUFSIZ];
    sprintf(tempSubject, SUBJECT, subject);
    strcat(finalMessage, tempSubject);
    char message[BUFSIZ];
    i = 0;
    while ((t = getchar()) != EOF) {
        if (t != '\n') {
            message[i++] = t;
        }
        while ((t = getchar()) != '\n' and t != EOF) {
            message[i++] = t;
        }
        if (t == '\n') {
            message[i++] = t;
        }
    }
    char tempMessage[BUFSIZ];
    sprintf(tempMessage, MESSAGE, message);
    strcat(finalMessage, tempMessage);
    strcat(finalMessage, ".\r\n");
    runSmtpCommand(finalMessage, s, 1);
    runSmtpCommand(QUIT, s, 0);
    close(s);
    return 0;
}