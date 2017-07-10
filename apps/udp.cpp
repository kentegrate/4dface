#include "vrphone.hpp"

int UDP_client_init(struct sockaddr_in *addr,int port)
{
  int s=socket(AF_INET,SOCK_DGRAM,0);
  if(s==-1){perror("socket");exit(1);}

  //  struct sockaddr_in addr;
  addr->sin_family=AF_INET;
  //  inet_aton(IP_addr,&addr.sin_addr);
  addr->sin_port=htons(port);
  return s;
}

void UDP_client_fini(int sock){
  close(sock);
}


int UDP_server_init(struct sockaddr_in* addr, int port)
{
  int ss=socket(AF_INET,SOCK_DGRAM,0);
  if(ss==-1){perror("socket");exit(1);}

  addr->sin_family=AF_INET;
  addr->sin_port=htons(port);
  addr->sin_addr.s_addr=INADDR_ANY;
  int bd=bind(ss,(struct sockaddr *)addr,sizeof(struct sockaddr));
  if(bd==-1){perror("bind");exit(1);}

  return ss;

}
int UDP_server_fini(int sock){
  close(sock);
}
