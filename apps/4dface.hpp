#pragma once
#ifndef FACE4D_HPP
#define FACE4D_HPP


#include "eos/fitting/fitting.hpp"
#include "eos/fitting/orthographic_camera_estimation_linear.hpp"
#include "eos/fitting/contour_correspondence.hpp"
#include "eos/fitting/closest_edge_fitting.hpp"
#include "eos/fitting/RenderingParameters.hpp"
#include "eos/render/utils.hpp"
#include "eos/render/render.hpp"
#include "eos/render/texture_extraction.hpp"

#include "rcr/model.hpp"
#include "cereal/cereal.hpp"

#include "glm/gtc/quaternion.hpp"

#include "Eigen/Dense"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include "boost/program_options.hpp"
#include "boost/filesystem.hpp"
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include "helpers.hpp"
using namespace eos;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
using cv::Mat;
using cv::Vec2f;
using cv::Vec3f;
using cv::Vec4f;
using cv::Rect;
using std::cout;
using std::endl;
using std::vector;
using std::string;

class Face4D{
public:
  void init();
  void getFrame(cv::Mat &img);
  void fini();
  morphablemodel::MorphableModel* morphable_model;
  core::LandmarkMapper* landmark_mapper;
  fitting::ModelContour* model_contour;
  fitting::ContourLandmarks* ibug_contour;
  rcr::detection_model* rcr_model;
  vector<morphablemodel::Blendshape>* blendshapes;
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


#endif // FACE4D_HPP
