#include "vrphone.hpp"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

void AudioInterface::init(){
  if(mode == MEDIA_SEND){
    sox_signalinfo_t signalinfo;
    signalinfo.rate = 48000;
    signalinfo.channels = 1;
    signalinfo.precision = 8;
    signalinfo.length = -1;
    signalinfo.mult = NULL;
  
    audio_buff = (sox_sample_t*)malloc(sizeof(sox_sample_t) * BUFF_SIZE);
    
    //buff_size = BUFF_SIZE;//sizeof(sox_sample_t) * BUFF_SIZE;
    buff_size = BUFF_SIZE;
    //    sox_init();
    printf("before init\n");
    fflush(stdout);
    
    //    sox_init();
    printf("init ok\n");
    fflush(stdout);
    //    ft = sox_open_read("default", &signalinfo, 0, "pulseaudio");
  }
  else{
    sox_signalinfo_t signalinfo;
    signalinfo.rate = 48000;
    signalinfo.channels = 1;
    signalinfo.precision = 8;
    signalinfo.length = -1;
    signalinfo.mult = NULL;
    
    audio_buff = (sox_sample_t*)malloc(sizeof(sox_sample_t) * BUFF_SIZE);
    //    buff_size = sizeof(audio_buff);//BUFF_SIZE;//sizeof(sox_sample_t) * BUFF_SIZE;
    buff_size = BUFF_SIZE;
    //
    printf("before init\n");
    fflush(stdout);
    
    //    sox_init();
    printf("init ok\n");
    fflush(stdout);
  
    //    ft = sox_open_write("default", &signalinfo, 0, "pulseaudio", 0, 0);
    read_size = 0;
  }
}

void AudioInterface::prepareSendMedia(){
  int n = sox_read(ft, audio_buff, buff_size);
  if(n == -1){
    perror("read");
  }
  send_size = n;
}
void AudioInterface::receiveMedia(int socket, struct sockaddr_in addr, bool wait_al){
  read_size = recv(socket, audio_buff, buff_size*4, 0);
  if(read_size == -1){
    perror("receive");
  }
}
void AudioInterface::sendMedia(int socket, struct sockaddr_in addr){
  sendto(socket, audio_buff, send_size*4, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr));
}
void AudioInterface::playRecvMedia(){
  if(read_size > 0)
    sox_write(ft, audio_buff, read_size/4);
}

void AudioInterface::fini(){
  free(buff);
}
