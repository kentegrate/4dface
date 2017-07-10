#include "vrphone.hpp"

void MediaInterface::receiveMedia(int socket, struct sockaddr_in addr, bool wait_all){
  printf("waiting size: %d", buff_size);
  fflush(stdout);
  socklen_t len = sizeof(addr);
  if(!wait_all){
    read_size = recvfrom(socket, buff, buff_size, 0, (struct sockaddr*)&addr, &len);
    printf("hogeho2\n");
    fflush(stdout);    
  }

  else{
    read_size = recvfrom(socket, buff, buff_size, MSG_WAITALL, (struct sockaddr*)&addr, &len);
    printf("hogeho3\n");        
  }
  if(read_size == -1){
    perror("receive");
  }
}

void MediaInterface::sendMedia(int socket, struct sockaddr_in addr){
  printf("sending size: %d", send_size);  
  sendto(socket, buff, send_size, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr));
}

void MediaInterface::fini(){

}
