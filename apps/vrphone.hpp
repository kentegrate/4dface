#pragma once
#ifndef VRPHONE_HPP
#define VRPHONE_HPP
#define BUFF_SIZE 4800

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "opencv2/core/core.hpp"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#define MEDIA_SEND 0
#define MEDIA_RECV 1
//#include "4dface.hpp"
namespace morphablemodel{
  class MorphableModel;
  class Blendshape;
  class EdgeTopology;
  
}
namespace core{
  class LandmarkMapper;
}
namespace fitting{
  class ModelContour;
  class ContourLandmarks;

}
namespace rcr{
  class detection_model;
  template <typename T>
  class LandmarkCollection;

}

class PcaCoefficientMerging;

class Face4D{
public:
  void init();
  void getFrame(cv::Mat &frame);
  void fini();
  morphablemodel::MorphableModel* morphable_model;
  core::LandmarkMapper* landmark_mapper;
  fitting::ModelContour* model_contour;
  fitting::ContourLandmarks* ibug_contour;
  rcr::detection_model* rcr_model;
  std::vector<morphablemodel::Blendshape>* blendshapes;
  morphablemodel::EdgeTopology* edge_topology;
  rcr::LandmarkCollection<cv::Vec2f>* current_landmarks;
  PcaCoefficientMerging* pca_shape_merging;
  
  cv::Mat frame, unmodified_frame;
  bool have_face;
  cv::Rect current_facebox;
  cv::Mat isomap;
  cv::CascadeClassifier face_cascade;
  cv::VideoCapture cap;
  
};

class MediaInterface{
public:
  virtual void init() = 0;
  virtual void receiveMedia(int socket, struct sockaddr_in addr, bool wait_all=true);
  virtual void sendMedia(int socket, struct sockaddr_in addr);
  virtual void fini();
  virtual void prepareSendMedia() = 0;
  virtual void playRecvMedia() = 0;
  uchar* buff;
  int buff_size;
  int mode;
  int read_size;
  int send_size;
};


class VideoInterface : public MediaInterface{
public:
  VideoInterface(int mode){
    this->mode = mode;
  }
  virtual void init();
  virtual void fini();
  virtual void receiveMedia(int socket, struct sockaddr_in addr, bool wait_all=true);
  virtual void sendMedia(int socket, struct sockaddr_in addr);
  virtual void prepareSendMedia();
  virtual void playRecvMedia();
  cv::Mat img;
  Face4D face;
};

class AudioInterface : public MediaInterface{
public:
  AudioInterface(int mode){
    this->mode = mode;
  }
  
  virtual void init();
  virtual void fini();    
  virtual void prepareSendMedia();
  virtual void playRecvMedia();
  int sox;
};

int UDP_client_init(struct sockaddr_in* addr, int port);
void UDP_client_fini(int sock);
int UDP_server_init(struct sockaddr_in* addr, int port);
int UDP_server_fini(int sock);


int TCP_client_init(char *IP_addr,int port);
void TCP_client_fini(int sock);
int TCP_server_init(int port,struct sockaddr_in *client);
void TCP_server_fini(int sock);



#endif// VRPHONE_HPP
