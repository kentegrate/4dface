#include "vrphone.hpp"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
void AudioInterface::init(){
  if(mode == MEDIA_SEND){
    FILE *fp=popen("rec -t raw -b 16 -c 1 -e s -r 44100 -", "r");
    if(fp == NULL){
      fprintf(stderr, "popen error");
    }
    sox = fileno(fp);
  }
  else{
    FILE *fp=popen("play -t raw -b 16 -c 1 -e s -r 44100 -", "w");
    if(fp == NULL){
      fprintf(stderr, "popen error");
    }
    sox = fileno(fp);
  }
  buff_size = sizeof(uchar)*BUFF_SIZE;  
  buff = (uchar*)malloc(buff_size);
}

void AudioInterface::prepareSendMedia(){
  int n = read(sox, buff, BUFF_SIZE);
  if(n == -1){
    perror("read");
  }
  send_size = n;
}

void AudioInterface::playRecvMedia(){
  write(sox, buff, read_size);
}

void AudioInterface::fini(){
  free(buff);
}
