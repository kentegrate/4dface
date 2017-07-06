#pragma once
#ifndef VRPHONE_HPP
#define VRPHONE_HPP
#define BUFF_SIZE 1024

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
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
  virtual void receiveMedia(int socket);
  virtual void sendMedia(int socket, size_t bytes);
  virtual void fini();
  virtual void prepareSendMedia() = 0;
  virtual void playRecvMedia() = 0;
  char buff[BUFF_SIZE];
  int mode;
  int read_size;
};


class VideoInterface : public MediaInterface{
public:
  VideoInterface(int mode){
    this->mode = mode;
  }
  void init();
  void fini();  
  void receiveMedia(int socket);
  void sendMedia(int socket, size_t bytes);
  void prepareSendMedia();
  void playRecvMedia();
  cv::Mat img;
  Face4D face;
  uchar* img_ptr;
  int img_size;
};

class AudioInterface : public MediaInterface{
public:
  AudioInterface(int mode){
    this->mode = mode;
  }
  
  void init();
  void prepareSendMedia();
  void playRecvMedia();
  int sox;
};

#endif// VRPHONE_HPP
