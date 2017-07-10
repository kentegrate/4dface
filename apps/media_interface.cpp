#include "vrphone.hpp"

void MediaInterface::receiveMedia(int socket, struct sockaddr_in addr, bool wait_all){
  read_size = recv(socket, buff, buff_size, 0);
  if(read_size == -1){
    perror("receive");
  }
}

void MediaInterface::sendMedia(int socket, struct sockaddr_in addr){
  sendto(socket, buff, send_size, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr));
}

void MediaInterface::fini(){

}
