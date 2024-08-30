#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <calcLib.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <chrono>


std::vector<std::string> split(std::string input_string,std::string delimiter);

int prepare_server_socket(std::string &ip, int port,int* ip_statuts);

char* math(std::string string, double a, double b);

std::string generate_calc_expression();

void printtime();

char buffer[1024];
int main_socket_fd = 0;
int client_socket_fd = 0;
int bytes_transferred;
fd_set socket_read_set;

char error_message[] = "ERROR\n";



int main(int argc, char *argv[]){

  initCalcLib();
  std::string delimiter = ":";
  std::vector<std::string> outputString = split(argv[1],":"); 
  std::string ip_address = "";
  int port;
  char timeo[] = "ERROR TO\n";

  if(outputString.size() > 2){
  port = atoi(outputString[outputString.size()-1].c_str());
  for(int i=0; i < 8 ; i++){
   ip_address = ip_address + outputString[i];}}
  else{
   port = atoi(outputString[1].c_str());
   ip_address = outputString[0];}

  int *ip_statuts = new int;

  int main_socket_fd = prepare_server_socket(ip_address ,port, ip_statuts);   
  FD_ZERO(&socket_read_set);
  FD_SET(main_socket_fd, &socket_read_set);

  while(1){
    int rc = select(main_socket_fd + 1, &socket_read_set, NULL, NULL, NULL);
    if(rc < 0){perror("error with select system call"); exit(1);}

    if(FD_ISSET(main_socket_fd, &socket_read_set)){
      struct sockaddr_in client_addr;
      struct sockaddr_in6 client_addr6;
      if(*ip_statuts == 1){
      socklen_t addrlen = sizeof(client_addr);
      client_socket_fd = accept(main_socket_fd, (struct sockaddr*)&client_addr, &addrlen);}
      else if(*ip_statuts == 2){
      socklen_t addrlen = sizeof(client_addr6);
      client_socket_fd = accept(main_socket_fd, (struct sockaddr*)&client_addr, &addrlen);}

      if(client_socket_fd < 0){
      perror("error with accept");exit(1);}

      struct timeval timeout;
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;

      if(setsockopt(client_socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1){{perror("setsockopt");exit(1);}}
      std::cout << std::endl;
      
      while(1){
        char intialMessage[] = "TEXT TCP 1.0\n";
        bytes_transferred = send(client_socket_fd, intialMessage, sizeof(intialMessage),0);
        if(bytes_transferred < 0){perror("error while sending data to client");close(client_socket_fd);exit(1);}
        char newline[] = "\n";
        bytes_transferred = send(client_socket_fd, newline, sizeof(newline),0);
        if(bytes_transferred < 0){perror("error while sending data to client");close(client_socket_fd);exit(1);}
         memset(buffer, 0, sizeof(buffer));
         bytes_transferred = recv(client_socket_fd, buffer, sizeof(buffer), 0);
         if(bytes_transferred < 0){
             if(errno == EAGAIN){
             bytes_transferred = send(client_socket_fd, timeo, sizeof(timeo),0);
                if(bytes_transferred < 0){perror("fail to send data");exit(1);}
                close(client_socket_fd);break;}
                else{perror("error while receving datat");close(client_socket_fd);exit(1);} }

        std::string receivedData(buffer);
        receivedData = receivedData.substr(0,bytes_transferred-1);

        if(receivedData == "OK"){
           std::string calString = generate_calc_expression();
           std::vector<std::string> inputVector = split(calString," ");
           calString = calString + "\n";
           std::cout << calString;
           double a = std::stod(inputVector[1]);
           double b = std::stod(inputVector[2]);
           char *result = math(inputVector[0], a, b);
           std::string calServerResult(result); 
           std::cout << result;
           bytes_transferred = send(client_socket_fd, calString.c_str(), strlen(calString.c_str()),0);
           if(bytes_transferred < 0){perror("error while sending data ");close(client_socket_fd);exit(1);}

           memset(buffer, 0, sizeof(buffer));
           bytes_transferred = recv(client_socket_fd, buffer, sizeof(buffer), 0);

           if(bytes_transferred < 0){
               if(errno == EAGAIN){
                bytes_transferred = send(client_socket_fd, timeo, sizeof(timeo),0);
                   if(bytes_transferred < 0){perror("fail to send data");exit(1);}
                   close(client_socket_fd);break;}
                   else{perror("error while receving data"); close(client_socket_fd);exit(1);}
                   close(client_socket_fd);break;}
           
           std::string receivedData(buffer);
           receivedData = receivedData.substr(0,bytes_transferred-1);
           receivedData = receivedData + "\n";
           if(receivedData == calServerResult){
            std::cout << "OK" << std::endl;
            char okchar[] = "OK\n";
            bytes_transferred = send(client_socket_fd, okchar, sizeof(okchar),0);
            if(bytes_transferred < 0){perror("error while sending data to client");close(client_socket_fd);exit(1);}
            close(client_socket_fd);
            break;}
            else {
             std::cout << error_message;
             bytes_transferred = send(client_socket_fd, error_message, sizeof(error_message),0);
             if(bytes_transferred < 0){perror("error while sending data to client"); close(client_socket_fd);exit(1);}
             close(client_socket_fd);
             break;}
              }
            else{
              std::cout << error_message;
              bytes_transferred = send(client_socket_fd, error_message, sizeof(error_message),0);
              close(client_socket_fd);
              break;}
            

      }
   }
  }







  return 0;
}


int prepare_server_socket(std::string &ip, int port,int* ip_statuts){
int socketfd; 
struct sockaddr_in ipv4;
struct sockaddr_in6 ipv6;
struct addrinfo hint, *output, *temp;
memset(&hint, 0, sizeof(hint));
hint.ai_family = AF_UNSPEC;
hint.ai_socktype = SOCK_STREAM;
int status = getaddrinfo(ip.c_str(), NULL, &hint, &output);
if(status != 0){
perror("error with getaddress info");exit(1);}

for(temp=output; temp != NULL;temp->ai_addr){
if(temp->ai_family == AF_INET){
ipv4.sin_family = AF_INET;
ipv4.sin_port = htons(port);
ipv4.sin_addr.s_addr = ((struct sockaddr_in*)temp->ai_addr)->sin_addr.s_addr;
socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if(socketfd > 0){
  if(bind(socketfd,(struct sockaddr*)&ipv4,sizeof(struct sockaddr)) < 0){perror("error with binding the ip address");exit(1);}
  std::cout << "Listening on " << ip << " port " << port << std::endl;
  *ip_statuts = 1;
   break;
}}
                                              
else if(temp->ai_family == AF_INET6){
ipv6.sin6_family = AF_INET6;
ipv6.sin6_port = htons(port);
ipv6.sin6_addr = ((struct sockaddr_in6*)temp->ai_addr)->sin6_addr;
socketfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
if(socketfd > 0){
  if(bind(socketfd,(struct sockaddr*)&ipv6,sizeof(struct sockaddr_in6)) < 0){perror("error with binding the ip address");exit(1);}
   std::cout << "Listening on " << ip << " port " << port << std::endl;
  *ip_statuts = 2;
   break;
}}}

if(*ip_statuts != 1 && *ip_statuts != 2){
  perror("error with socket");
  exit(1);}

if(listen(socketfd, 5) < 0){perror("error with listen function");exit(1);}
  
freeaddrinfo(output);
return socketfd;
}


char* math(std::string string, double a, double b) {
    char* str = new char[30]; 
    if (str == NULL) {
        return NULL; 
    }

    if (string == "fsub") {
        double c = a - b;
        sprintf(str, "%8.8g\n", c);
    } else if (string == "fmul") {
        double c = a * b;
        sprintf(str, "%8.8g\n", c);
    } else if (string == "fadd") {
        double c = a + b;
       sprintf(str, "%8.8g\n", c);
    } else if (string == "fdiv") {
        if (b != 0) {
            double c = a / b;
            sprintf(str, "%8.8g\n", c);
        } else {
            strcpy(str, "Error: Division by zero\n");
        }
    } else if (string == "sub") {
        int c = (int)(a - b); // Convert result to integer for "sub"
        sprintf(str, "%d\n", c);
    } else if (string == "mul") {
        int c = (int)(a * b); // Convert result to integer for "mul"
        sprintf(str, "%d\n", c);
    } else if (string == "add") {
        int c = (int)(a + b); // Convert result to integer for "add"
        sprintf(str, "%d\n", c);
    } else if (string == "div") {
        if (b != 0) {
            int c = (int)(a / b); // Convert result to integer for "div"
            sprintf(str, "%d\n", c);
        } else {
            strcpy(str, "Error: Division by zero\n");
        }
    } else {
        strcpy(str, "Error: Invalid operation\n");
    }

    return str;
}

std::vector<std::string> split(std::string input_string,std::string delimiter){
std::vector<std::string> result_strings;
std::string temp;

for(int i=0; i < static_cast<int>(input_string.length());i++){
  int  count = 0;
  if(input_string[i] == delimiter[0]){
        count++;
        result_strings.push_back(temp);
        temp  = "";
    }
  else{
        temp = temp +  input_string[i];
         }

  if(count==0 && (i == static_cast<int>(input_string.length()-1))){
         result_strings.push_back(temp);}               }

  if(result_strings.size() < 2){
    std::cout << "ERROR" << std::endl;
    exit(1);
  }



return result_strings;
}


std::string generate_calc_expression(){

std::string calString;

double float1,float2;
int integer1,integer2;

char *oper;

oper = randomType();

integer1 = randomInt();
integer2 = randomInt();

float1 = randomFloat();
float2 = randomFloat();

    if(strcmp(oper,"fadd")==0){
      calString = "fadd " + std::to_string(float1) + " " + std::to_string(float2);
    } else if (strcmp(oper, "fsub")==0){
       calString = "fsub " + std::to_string(float1) + " " + std::to_string(float2);
    } else if (strcmp(oper, "fmul")==0){
       calString = "fmul " + std::to_string(float1) + " " + std::to_string(float2);
    } else if (strcmp(oper, "fdiv")==0){
       calString = "fdiv " + std::to_string(float1) + " " + std::to_string(float2);
    }
    else if(strcmp(oper,"add")==0){
       calString = "add " + std::to_string(integer1) + " " + std::to_string(integer2);
    } else if (strcmp(oper, "sub")==0){
       calString = "sub " + std::to_string(integer1) + " " + std::to_string(integer2);
    } else if (strcmp(oper, "mul")==0){
      calString = "mul " + std::to_string(integer1) + " " + std::to_string(integer2);
    } else if (strcmp(oper, "div")==0){
      calString = "div " + std::to_string(integer1) + " " + std::to_string(integer2);
    }

return calString;
}