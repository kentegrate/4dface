#include "vrphone.hpp"

void VideoInterface::init(){
  img = cv::Mat::zeros(400, 800, CV_8UC4);
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
  std::vector<uchar> img_buff;
  int img_size = 0;
  while(1){
    uchar tiny_buff[BUFF_SIZE];
    int n = recv(socket, tiny_buff, BUFF_SIZE, 0);
    img_size += n;
    if(n==0)break;
    if(n==-1)break;
    for(int i = 0; i < n; i++){
      img_buff.push_back(tiny_buff[i]);
    }
    if(n != BUFF_SIZE)break;
  }
  cv::Mat img_tmp = imdecode(cv::Mat(img_buff), CV_LOAD_IMAGE_COLOR);
  if(img_tmp.rows > 0 && img_tmp.cols > 0){
    img = img_tmp.clone();
  }
}

void VideoInterface::sendMedia(int socket, struct sockaddr_in addr){
  uchar* tiny_buffer;
  std::vector<uchar> img_buff;
  std::vector<int> param = std::vector<int>(2);
  param[0] = CV_IMWRITE_JPEG_QUALITY;
  param[1] = 40;
  imencode(".jpg", img, img_buff, param);
  int img_bytes = img_buff.size();
  int current_idx = 0;
  while(1){
    int sending_bytes = img_bytes - current_idx;
    if(sending_bytes <= 0)break;
    if(sending_bytes > BUFF_SIZE){
      sending_bytes = BUFF_SIZE;
    }
    tiny_buffer = &img_buff[0] + current_idx;
    sendto(socket, tiny_buffer, sending_bytes, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr));
    current_idx += sending_bytes;
  }

}

void VideoInterface::prepareSendMedia(){
  face.getFrame(img); //TODO : make 4d face rendering async.
  
  buff = img.data;
  send_size = img.total() * img.elemSize();
    cv::imshow("VR Phone Server", img);
    auto key = cv::waitKey(1);  
}

void VideoInterface::playRecvMedia(){
  cv::imshow("VR Phone Client", img);
  auto key = cv::waitKey(1);  
}

