#include<iostream>
#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h> // for sigaction() function
#include <string.h>
#include "vrphone.hpp"

#define SERVER_TCP_PORT 50000
#define SERVER_VIDEO_UDP_PORT 50001
#define CLIENT_VIDEO_UDP_PORT 50002
#define SERVER_AUDIO_UDP_PORT 50003
#define CLIENT_AUDIO_UDP_PORT 50004

#define SERVER_MODE 0
#define CLIENT_MODE 1

int signaled = 0;
int op_mode;

struct sockaddr_in opponent_addr;

void threaded_send(struct sockaddr* dest_addr, int port, MediaInterface *media){
  int send_sock = UDP_client_init(dest_addr, port);
  media->init();
  while(!signaled){
    media->prepareSendMedia();
    media->sendMedia(send_sock);
  }
  media->fini();  
  UDP_client_fini(send_sock);
  std::cout << "thread send end"<<std::endl;
}

void threaded_recv(int port, MediaInterface *media){
  media->init();
  int recv_sock = UDP_server_init(port);
  while(!signaled){
    media->receiveMedia(recv_sock);
    media->playRecvMedia();
  }
  media->fini();
  UDP_server_fini(recv_sock);
  std::cout << "thread recv end"<<std::endl;  
}

void sighandler(int sig, siginfo_t * singinfo, void* context){
  signaled = 1;
}

void install_sig_hooks(){
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_sigaction = sighandler;
  action.sa_flags = SA_SIGINFO;
  sigaction(SIGINT, &action, NULL);
}

int main(int argc, char** argv){
  if(argc < 2){
    printf("usage: ./vrphone s \n ./vrphone c [server ip addr] \n");
    exit(1);
  }
  

  std::string option(argv[1]);
  
  if (option == "s"){//in server mode
    printf("starting in server mode.\n");
    op_mode = SERVER_MODE;

    int tcp_socket = TCP_server_init(SERVER_TCP_PORT, &opponent_addr);
    TCP_fini(tcp_socket);
  }
  else{//in client mode
    char* server_ip = argv[2];    
    printf("starting in client mode.\n");
    op_mode = CLIENT_MODE;
    int tcp_socket = TCP_client_init(server_ip, SERVER_TCP_PORT);
    TCP_fini(tcp_socket);
  }

  VideoInterface video_send(MEDIA_SEND);  
  VideoInterface video_recv(MEDIA_RECV);

  AudioInterface audio_send(MEDIA_SEND);
  AudioInterface audio_recv(MEDIA_RECV);



  std::thread video_send_thread(threaded_send, (struct sockaddr*)&opponent_addr,
				op_mode == CLIENT_MODE ? SERVER_VIDEO_UDP_PORT : CLIENT_VIDEO_UDP_PORT,
				&video_send);
				
  /*  std::thread video_recv_thread(threaded_recv,
				op_mode == SERVER_MODE ? SERVER_VIDEO_UDP_PORT : CLIENT_VIDEO_UDP_PORT,
				&video_recv);*/

  std::thread audio_send_thread(threaded_send, (struct sockaddr*)&opponent_addr,
				op_mode == CLIENT_MODE ? SERVER_AUDIO_UDP_PORT : CLIENT_AUDIO_UDP_PORT,
				&audio_send);
  /*				
  std::thread audio_recv_thread(threaded_recv,
				op_mode == SERVER_MODE ? SERVER_AUDIO_UDP_PORT : CLIENT_AUDIO_UDP_PORT,
				&audio_recv);*/
  
  install_sig_hooks();
  video_send_thread.join();
  //  video_recv_thread.join();
  /*  audio_send_thread.join();
      audio_recv_thread.join();*/
  
  return 0;
}
