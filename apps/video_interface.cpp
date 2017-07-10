#include "vrphone.hpp"

void VideoInterface::init(){
  img = cv::Mat::zeros(400, 800, CV_8UC4);
  printf("mat size %d \n", img.total() * img.elemSize());
  buff = img.data;
  buff_size = img.total() * img.elemSize();

  if(mode == MEDIA_SEND){
    face.init();
    cv::namedWindow("VR Phone Server", 1);
  }
  else{
    cv::namedWindow("VR Phone Client", 1);
  }
}
void VideoInterface::fini(){
  face.fini();
}

void VideoInterface::receiveMedia(int socket, struct sockaddr_in addr, bool wait_all){
  uchar* tiny_buffer;  
  for(int i = 0; i < 30; i++){
    tiny_buffer = img.data + (buff_size/30)*i;
    recv(socket, tiny_buffer, buff_size/30, 0);
  }
}

void VideoInterface::sendMedia(int socket, struct sockaddr_in addr){
  uchar* tiny_buffer;
  for(int i = 0; i < 30; i++){
    tiny_buffer = img.data + (send_size/30)*i;
    sendto(socket, tiny_buffer, send_size/30, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr));
    
  }
}

void VideoInterface::prepareSendMedia(){
  face.getFrame(img); //TODO : make 4d face rendering async.
  buff = img.data;
  send_size = img.total() * img.elemSize();

  printf("height %d, width %d, depth %d", img.rows, img.cols, img.depth());
    cv::imshow("VR Phone Server", img);
    auto key = cv::waitKey(1);  
}

void VideoInterface::playRecvMedia(){
  cv::imshow("VR Phone Client", img);
  auto key = cv::waitKey(1);  
}

