#include<iostream>
#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h> // for sigaction() function
#include <string.h>
#include "vrphone.hpp"
#include <stdlib.h>

#define SERVER_TCP_PORT 50000
#define SERVER_VIDEO_UDP_PORT 50012
#define CLIENT_VIDEO_UDP_PORT 50013
#define SERVER_AUDIO_UDP_PORT 50003
#define CLIENT_AUDIO_UDP_PORT 50004

#define SERVER_MODE 0
#define CLIENT_MODE 1

int signaled = 0;
int op_mode;

struct sockaddr_in opponent_addr;

void threaded_send(struct sockaddr_in* addr, int port, MediaInterface *media){

  media->init();
  int send_sock = UDP_client_init(addr, port);  
  while(!signaled){

    media->prepareSendMedia();
    media->sendMedia(send_sock, *addr);

  }
  UDP_client_fini(send_sock);      
  media->fini();  

  std::cout << "thread send end"<<std::endl;
}

void threaded_recv(int port, MediaInterface *media){
  media->init();
  struct sockaddr_in addr;
  int recv_sock = UDP_server_init(&addr, port);
   while(!signaled){
     media->receiveMedia(recv_sock, addr);
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
  std::string option2(argv[2]);  
  if (option == "s"){//in server mode
    printf("starting in server mode.\n");
    op_mode = SERVER_MODE;
    printf("awaiting connection.\n");
    int tcp_socket = TCP_server_init(SERVER_TCP_PORT, &opponent_addr);
    TCP_server_fini(tcp_socket);
    printf("TCP connection finished\n");
  }
  else{
    printf("starting in client mode.\n");
    op_mode = CLIENT_MODE;
    char *ip = "192.168.100.109";
    //    char *ip = "127.0.0.1";
    inet_aton(ip, &opponent_addr.sin_addr);
    printf("starting connection.\n");
    int tcp_socket = TCP_client_init(ip, SERVER_TCP_PORT);
    
    TCP_client_fini(tcp_socket);
    printf("TCP connection finished\n");

  }


  VideoInterface video_send(MEDIA_SEND);  
  VideoInterface video_recv(MEDIA_RECV);

  AudioInterface audio_send(MEDIA_SEND);
  AudioInterface audio_recv(MEDIA_RECV);
  //  install_sig_hooks();
  //    if(false){
  if(option2 == "v"){  

    std::thread video_send_thread(threaded_send, &opponent_addr,
				op_mode == CLIENT_MODE ? SERVER_VIDEO_UDP_PORT : CLIENT_VIDEO_UDP_PORT,
				&video_send);
    std::thread video_recv_thread(threaded_recv,
				op_mode == SERVER_MODE ? SERVER_VIDEO_UDP_PORT : CLIENT_VIDEO_UDP_PORT,
				&video_recv);
    video_send_thread.join();
    video_recv_thread.join();
    
  }
  else{
    sox_init();
    sox_signalinfo_t signalinfo;
    signalinfo.rate = 48000;
    signalinfo.channels = 1;
    signalinfo.precision = 8;
    signalinfo.length = -1;
    signalinfo.mult = NULL;
  
    audio_send.ft = sox_open_read("default", &signalinfo, 0, "pulseaudio");
    audio_recv.ft = sox_open_write("default", &signalinfo, 0, "pulseaudio", 0,0);
    std::thread audio_send_thread(threaded_send, &opponent_addr,
				  op_mode == CLIENT_MODE ? SERVER_AUDIO_UDP_PORT : CLIENT_AUDIO_UDP_PORT,
				  &audio_send);
    std::thread audio_recv_thread(threaded_recv,
				  op_mode == SERVER_MODE ? SERVER_AUDIO_UDP_PORT : CLIENT_AUDIO_UDP_PORT,
				  &audio_recv);
    audio_send_thread.join();  
    audio_recv_thread.join();
    sox_close(audio_send.ft);
    sox_close(audio_recv.ft);
    sox_quit();
  }

          



  
  return 0;
}
