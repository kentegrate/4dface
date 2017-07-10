#include "vrphone.hpp"

void VideoInterface::init(){
  img = cv::Mat::zeros(100, 200, CV_8UC4);
  printf("mat size %d \n", img.total() * img.elemSize());
  buff = img.data;
  buff_size = 1000;//img.total() * img.elemSize();


  if(mode == MEDIA_SEND){
    face.init();
    cv::namedWindow("VR Phone Server", 1);
  }
  else{
    //    cv::namedWindow("VR Phone Client", 1);
  }
}
void VideoInterface::fini(){
  face.fini();
}

void VideoInterface::receiveMedia(int socket, struct sockaddr_in addr, bool wait_all){
  printf("hogehogehogefooo\n");
  MediaInterface::receiveMedia(socket, addr, false);
  printf("hogehogehogefooo2\n");  
}

void VideoInterface::prepareSendMedia(){
  face.getFrame(img); //TODO : make 4d face rendering async.
  send_size = img.total() * img.elemSize();
  printf("height %d, width %d, depth %d", img.rows, img.cols, img.depth());
  cv::imshow("VR Phone Server", img);
  auto key = cv::waitKey(1);  
}

void VideoInterface::playRecvMedia(){
  //  cv::imshow("VR Phone Client", img);
  //  auto key = cv::waitKey(1);  
}

