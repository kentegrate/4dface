#include "vrphone.hpp"

void VideoInterface::init(){
  img = cv::Mat::zeros(480, 640, CV_8UC3);
  img_size = img.total() * img.elemSize();
  img_ptr = img.data;

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
void VideoInterface::sendMedia(int socket, size_t bytes){
  send(socket, img_ptr, img_size, 0);
}
void VideoInterface::receiveMedia(int socket){
  int n;
  if((n = recv(socket, img_ptr, img_size, MSG_WAITALL)) == -1){
    perror("receive");
  }
}

void VideoInterface::prepareSendMedia(){
  face.getFrame(img); //TODO : make 4d face rendering async.
  cv::imshow("VR Phone Server", img);
  auto key = cv::waitKey(1);  
}

void VideoInterface::playRecvMedia(){
  cv::imshow("VR Phone Client", img);
  auto key = cv::waitKey(1);  
}

