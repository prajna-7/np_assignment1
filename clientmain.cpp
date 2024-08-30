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


std::vector<std::string> split(std::string input_string,std::string delimiter);

int prepare_server_socket(std::string &ip, int port,int* ip_statuts);


std::vector<std::string> result_strings;


std::string temp;


std::string recvBuffer(int sockerfd);


void sendBuffer(int socketfd,const char* mess);


char* math(std::string string, double a, double b);



int main(int argc, char *argv[]){
std::string delimiter = ":";

std::vector<std::string> outputString = split(argv[1],":"); 

std::string ip_address = "";

int port;

if(outputString.size() > 2){
  port = atoi(outputString[outputString.size()-1].c_str());
  for(int i=0; i < 8 ; i++){
  if(i > 0){
   ip_address = ip_address + ":" + outputString[i];}
   else{
   ip_address = ip_address + outputString[i];}
   }
  }

else{
port = atoi(outputString[1].c_str());
ip_address = outputString[0];
}

#ifdef DEBUG 
  std::cout << "Host Name is " << ip_address << " port " << port << std::endl;
#endif



int *ip_statuts = new int;

int socketfd = prepare_server_socket(ip_address ,port, ip_statuts);

#ifdef DEBUG 
  if(*ip_statuts == 1){
  std::cout << "IP type IPV4" << std::endl;}
  else if(*ip_statuts == 2){
  std::cout << "IP type is IPV6" << std::endl;
  }
#endif

    std::string okChar = "OK\n";
    std::string firstmes = recvBuffer(socketfd);
    if(firstmes == "TEXT TCP 1.0\n\n"){
        sendBuffer(socketfd,okChar.c_str());
        std::string message = recvBuffer(socketfd);
        std::vector<std::string> inputVector = split(message," ");
        double a = std::stod(inputVector[1]);
        double b = std::stod(inputVector[2]);
        char *result = math(inputVector[0], a, b);
        sendBuffer(socketfd, result);
        recvBuffer(socketfd);}
    else{
        std::cout << "error " << std::endl;
    }
    close(socketfd); 
    
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
std::cout << "There is problem in getting getaddrinfo" << std::endl;}

for(temp=output; temp != NULL;temp->ai_addr){

if(temp->ai_family == AF_INET){
ipv4.sin_family = AF_INET;
ipv4.sin_port = htons(port);
ipv4.sin_addr.s_addr = ((struct sockaddr_in*)temp->ai_addr)->sin_addr.s_addr;
socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if(socketfd > 0){
   if(connect(socketfd, (struct sockaddr*)&ipv4, sizeof(struct sockaddr)) < 0){perror("error with connect");close(socketfd);fflush(stderr);exit(1);}
    std::cout << "Connected to " << ip << ":" << port << std::endl;
  *ip_statuts = 1;
   break;
}}
                                              
else if(temp->ai_family == AF_INET6){
ipv6.sin6_family = AF_INET6;
ipv6.sin6_port = htons(port);
ipv6.sin6_addr = ((struct sockaddr_in6*)temp->ai_addr)->sin6_addr;
socketfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
if(socketfd > 0){
   if(connect(socketfd, (struct sockaddr*)&ipv6, sizeof(struct sockaddr_in6)) < 0){perror("error with connect");close(socketfd);fflush(stderr);exit(1);}
   std::cout << "Connected to " << ip << ":" << port << std::endl;
  *ip_statuts = 2;
   break;
}}}

if(*ip_statuts != 1 && *ip_statuts != 2){perror("error with socket");close(socketfd);exit(1);}
  
freeaddrinfo(output);
return socketfd;
}


void sendBuffer(int socketfd,const char* mess){

int bytes_transferred = send(socketfd, mess, strlen(mess),0);
if(bytes_transferred < 0){perror("send");close(socketfd);exit(1);}
else{std::cout << mess;}

}

std::string recvBuffer(int socketfd){

int bufferSize = 1024;
char recv_buffer[bufferSize];
memset(recv_buffer, 0, bufferSize);
int bytes_transferred = recv(socketfd, recv_buffer, bufferSize,0);
std::string recvBuf(recv_buffer);
if(bytes_transferred < 0){perror("error with recv");close(socketfd);exit(1);}
else{std::cout << recv_buffer;}

return recvBuf;
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



return result_strings;
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
        int c = (int)(a - b); 
        sprintf(str, "%d\n", c);
    } else if (string == "mul") {
        int c = (int)(a * b);
        sprintf(str, "%d\n", c);
    } else if (string == "add") {
        int c = (int)(a + b); 
        sprintf(str, "%d\n", c);
    } else if (string == "div") {
        if (b != 0) {
            int c = (int)(a / b); 
            sprintf(str, "%d\n", c);
        } else {
            strcpy(str, "Error: Division by zero\n");
        }
    } else {
        strcpy(str, "Error: Invalid operation\n");
    }

    return str;
}


