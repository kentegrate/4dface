#include "vrphone.hpp"

void MediaInterface::receiveMedia(int socket){
  int read_size = recv(socket, buff, BUFF_SIZE, 0);
  if(read_size == -1){
    perror("read");
  }
}

void MediaInterface::sendMedia(int socket, size_t bytes){
  send(socket, buff, bytes, 0);
}

void MediaInterface::fini(){

}
