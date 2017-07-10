#include "vrphone.hpp"
int TCP_client_init(char *IP_addr,int port)
{
  int s=socket(AF_INET,SOCK_STREAM,0);
  if(s==-1){perror("socket");exit(1);}

  struct sockaddr_in addr;
  addr.sin_family=AF_INET;
  addr.sin_addr.s_addr=inet_addr(IP_addr);
  addr.sin_port=htons(port);
  int ret=connect(s,(struct sockaddr *)&addr,sizeof(addr));
  if(ret==-1){perror("connect");exit(1);}
  
  return s;
}

void TCP_client_fini(int sock){
  close(sock);
}

int TCP_server_init(int port,struct sockaddr_in *client)
{
  int ss=socket(AF_INET,SOCK_STREAM,0);
  if(ss==-1){perror("socket");exit(1);}

  struct sockaddr_in addr;
  addr.sin_family=AF_INET;
  addr.sin_port=htons(port);
  addr.sin_addr.s_addr=INADDR_ANY;
  int bd=bind(ss,(struct sockaddr *)&addr,sizeof(addr));
  if(bd==-1){perror("bind");exit(1);}

  listen(ss,10);
  
  socklen_t len=sizeof(struct sockaddr_in);
  int s=accept(ss,(struct sockaddr *)client,&len);
  close(ss);
  if(s==-1){perror("accept");exit(1);}

  return s;
}

void TCP_server_fini(int sock){
  close(sock);
}
