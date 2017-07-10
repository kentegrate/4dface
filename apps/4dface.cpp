/*
 * 4dface: Real-time 3D face tracking and reconstruction from 2D video.
 *
 * File: apps/4dface.cpp
 *
 * Copyright 2015, 2016 Patrik Huber
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "4dface.hpp"
#include <vector>
#include <iostream>

bool isomap_ready = false;
Mat global_isomap;
float parallax_coeff = 0;
void extract_texture_async(render::Mesh mesh, Mat affine_cam, Mat unmodified_frame, bool compute_view_angle, render::TextureInterpolation mapping_type, int isomap_resolution){
  render::Mesh _mesh = mesh;
  Mat _affine_cam = affine_cam.clone();
  Mat _unmodified_frame = unmodified_frame.clone();
  global_isomap = render::extract_texture(_mesh, _affine_cam, _unmodified_frame, compute_view_angle, mapping_type, isomap_resolution);
  isomap_ready = true;
}
void render_anaglyph(render::Mesh mesh, glm::tmat4x4<float> model_view_matrix, glm::tmat4x4<float> projection_matrix,
		     int viewport_width, int viewport_height, const boost::optional<render::Texture> texture, Mat &rendered){
  Mat render_left, render_right;
  model_view_matrix[3][0] = parallax_coeff;
  std::tie(render_left, std::ignore) = render::render(mesh, model_view_matrix, projection_matrix, viewport_width,
						      viewport_height, texture, true, false, false);
  model_view_matrix[3][0] = -parallax_coeff;
  std::tie(render_right, std::ignore) = render::render(mesh, model_view_matrix, projection_matrix, viewport_width,
						      viewport_height, texture, true, false, false);  
  //  vector<Mat> left_channels;
  //  vector<Mat> right_channels;  
  
  /*  cv::split(render_left, left_channels);
  cv::split(render_right, right_channels);
  std::vector<Mat> merging_channels{right_channels[0], right_channels[1], left_channels[2]};
  cv::merge(merging_channels, rendered);*/
  cv::hconcat(render_left, render_right, rendered);
}

void draw_axes_topright(float r_x, float r_y, float r_z, cv::Mat image);

/**
 * This app demonstrates facial landmark tracking, estimation of the 3D pose
 * and fitting of the shape model of a 3D Morphable Model from a video stream,
 * and merging of the face texture.
 */
void Face4D::init()
{
  morphable_model = new morphablemodel::MorphableModel;
  landmark_mapper = new core::LandmarkMapper;
  model_contour = new fitting::ModelContour;
  ibug_contour = new fitting::ContourLandmarks;
  rcr_model = new rcr::detection_model;
  blendshapes = new vector<morphablemodel::Blendshape>;
  edge_topology = new morphablemodel::EdgeTopology;
  current_landmarks = new rcr::LandmarkCollection<cv::Vec2f>;
  pca_shape_merging = new PcaCoefficientMerging;

  
  have_face = false;
  std::string abs_path("/home/denjo/4dface/install/share/");
  fs::path modelfile("sfm_shape_3448.bin");
  fs::path inputvideo;
  fs::path facedetector("haarcascade_frontalface_alt2.xml");
  fs::path landmarkdetector("face_landmarks_model_rcr_68.bin");
  fs::path mappingsfile("ibug2did.txt");
  fs::path contourfile("model_contours.json");
  fs::path edgetopologyfile("sfm_3448_edge_topology.json");
  fs::path blendshapesfile("expression_blendshapes_3448.bin");
  parallax_coeff = 20;
  std::cout << fs::absolute(modelfile, abs_path).string() << std::endl;
  *morphable_model = morphablemodel::load_model(fs::absolute(modelfile, abs_path).string());
  *landmark_mapper = core::LandmarkMapper(fs::absolute(mappingsfile,abs_path));
  *model_contour = fitting::ModelContour::load(fs::absolute(contourfile, abs_path).string());
  *ibug_contour = fitting::ContourLandmarks::load(fs::absolute(mappingsfile, abs_path).string());

  // Load the landmark detection model:
  try {
    *rcr_model = rcr::load_detection_model(fs::absolute(landmarkdetector, abs_path).string());
  }
  catch (const cereal::Exception& e) {
    cout << "Error reading the RCR model " << landmarkdetector << ": " << e.what() << endl;
    //    return EXIT_FAILURE;
  }

  // Load the face detector from OpenCV:

  if (!face_cascade.load(fs::absolute(facedetector, abs_path).string()))
    {
      cout << "Error loading the face detector " << facedetector << "." << endl;
      //      return EXIT_FAILURE;
    }
  printf("hgo3\n");  
  fflush(stdout);  

  if (inputvideo.empty()) {
    cap.open(0); // no file given, open the default camera
  }
  else {
    cap.open(inputvideo.string());
  }
  if (!cap.isOpened()) {
    cout << "Couldn't open the given file or camera 0." << endl;
    //    return EXIT_FAILURE;
  }

  *blendshapes = morphablemodel::load_blendshapes(fs::absolute(blendshapesfile, abs_path).string());

  *edge_topology = morphablemodel::load_edge_topology(fs::absolute(edgetopologyfile, abs_path).string());

  cv::namedWindow("video", 1);
  //  cv::namedWindow("render", 1);
}
void Face4D::getFrame(Mat &img)
{
  cap >> frame; // get a new frame from camera
  if (frame.empty()) { // stop if we're at the end of the video
    std::cout << "end of video" << std::endl;
  }

  // We do a quick check if the current face's width is <= 50 pixel. If it is, we re-initialise the tracking with the face detector.
  if (have_face && get_enclosing_bbox(rcr::to_row(*current_landmarks)).width <= 50) {
    cout << "Reinitialising because the face bounding-box width is <= 50 px" << endl;
    have_face = false;
  }

  unmodified_frame = frame.clone();

  if (!have_face) {
    // Run the face detector and obtain the initial estimate using the mean landmarks:
    vector<Rect> detected_faces;
    face_cascade.detectMultiScale(unmodified_frame, detected_faces, 1.2, 2, 0, cv::Size(110, 110));
    if (detected_faces.empty()) {
      cv::imshow("video", frame);
      cv::waitKey(30);
      return;
    }
    cv::rectangle(frame, detected_faces[0], { 255, 0, 0 });
    // Rescale the V&J facebox to make it more like an ibug-facebox:
    // (also make sure the bounding box is square, V&J's is square)
    Rect ibug_facebox = rescale_facebox(detected_faces[0], 0.85, 0.2);

    *current_landmarks = rcr_model->detect(unmodified_frame, ibug_facebox);
    rcr::draw_landmarks(frame, *current_landmarks, { 0, 0, 255 }); // red, initial landmarks

    have_face = true;
  }
  else {
    // We already have a face - track and initialise using the enclosing bounding
    // box from the landmarks from the last frame:
    auto enclosing_bbox = get_enclosing_bbox(rcr::to_row(*current_landmarks));
    enclosing_bbox = make_bbox_square(enclosing_bbox);
    *current_landmarks = rcr_model->detect(unmodified_frame, enclosing_bbox);
    rcr::draw_landmarks(frame, *current_landmarks, { 255, 0, 0 }); // blue, the new optimised landmarks
  }

  // Fit the 3DMM:

  fitting::RenderingParameters rendering_params;
  vector<float> shape_coefficients, blendshape_coefficients;
  vector<Vec2f> image_points;
  render::Mesh mesh; 
  PerformanceMonitor performance; 
  performance.start(__LINE__);
  std::tie(mesh, rendering_params) = fitting::fit_shape_and_pose(*morphable_model, *blendshapes, rcr_to_eos_landmark_collection(*current_landmarks), *landmark_mapper, unmodified_frame.cols, unmodified_frame.rows, *edge_topology, *ibug_contour, *model_contour, 2, 5, 15.0f, boost::none, shape_coefficients, blendshape_coefficients, image_points);
  performance.lap(__LINE__);	
  // Draw the 3D pose of the face:
  draw_axes_topright(glm::eulerAngles(rendering_params.get_rotation())[0], glm::eulerAngles(rendering_params.get_rotation())[1], glm::eulerAngles(rendering_params.get_rotation())[2], frame);
  performance.lap(__LINE__);	
  // Wireframe rendering of mesh of this frame (non-averaged):
  draw_wireframe(frame, mesh, rendering_params.get_modelview(), rendering_params.get_projection(), fitting::get_opencv_viewport(frame.cols, frame.rows));
  performance.lap(__LINE__);	
  // Extract the texture using the fitted mesh from this frame:
  Mat affine_cam = fitting::get_3x4_affine_camera_matrix(rendering_params, frame.cols, frame.rows);
  performance.lap(__LINE__);
		
  if(isomap.empty()){
    boost::unique_future<void> extract =
      boost::async(boost::launch::async,
		   boost::bind(extract_texture_async, mesh, affine_cam,
			       unmodified_frame, true,
			       render::TextureInterpolation::NearestNeighbour, 512));
    extract.wait();
  }		

  if(isomap_ready){
    isomap_ready = false;
    isomap = global_isomap.clone();
    boost::unique_future<void> extract =
      boost::async(boost::launch::async,
		   boost::bind(extract_texture_async, mesh, affine_cam,
			       unmodified_frame, true,
			       render::TextureInterpolation::NearestNeighbour, 512));
		  
  }
  //		  isomap = render::extract_texture(mesh, affine_cam, unmodified_frame, true, render::TextureInterpolation::NearestNeighbour, 512);
		
  performance.lap(__LINE__);	
  // Merge the isomaps - add the current one to the already merged ones:
  //		Mat merged_isomap = isomap_averaging.add_and_merge(isomap);
  // Same for the shape:
  shape_coefficients = pca_shape_merging->add_and_merge(shape_coefficients);
  auto merged_shape = morphable_model->get_shape_model().draw_sample(shape_coefficients) + morphablemodel::to_matrix(*blendshapes) * Mat(blendshape_coefficients);
  render::Mesh merged_mesh = morphablemodel::sample_to_mesh(merged_shape, morphable_model->get_color_model().get_mean(), morphable_model->get_shape_model().get_triangle_list(), morphable_model->get_color_model().get_triangle_list(), morphable_model->get_texture_coordinates());
  performance.lap(__LINE__);	
  // Render the model in a separate window using the estimated pose, shape and merged texture:
  Mat rendering;
  auto modelview_no_translation = rendering_params.get_modelview();
  modelview_no_translation[3][0] = 20;
  modelview_no_translation[3][1] = 0;
  render_anaglyph(merged_mesh, modelview_no_translation, glm::ortho(-130.0f, 130.0f, -130.0f, 130.0f), 400, 400, render::create_mipmapped_texture(isomap), rendering);
  
  performance.stop(__LINE__);
  //  cv::imshow("render", rendering);
  cv::imshow("video", frame);
  img = rendering.clone();
  auto key = cv::waitKey(1);
}

void Face4D::fini(){
  delete morphable_model;
  delete landmark_mapper;
  delete model_contour;
  delete ibug_contour;
  delete rcr_model;
  delete blendshapes;
  delete edge_topology;
  delete current_landmarks;
  delete pca_shape_merging;
}

//return EXIT_SUCCESS;


/**
 * @brief Draws 3D axes onto the top-right corner of the image. The
 * axes are oriented corresponding to the given angles.
 *
 * @param[in] r_x Pitch angle, in radians.
 * @param[in] r_y Yaw angle, in radians.
 * @param[in] r_z Roll angle, in radians.
 * @param[in] image The image to draw onto.
 */
void draw_axes_topright(float r_x, float r_y, float r_z, cv::Mat image)
{
	const glm::vec3 origin(0.0f, 0.0f, 0.0f);
	const glm::vec3 x_axis(1.0f, 0.0f, 0.0f);
	const glm::vec3 y_axis(0.0f, 1.0f, 0.0f);
	const glm::vec3 z_axis(0.0f, 0.0f, 1.0f);

	const auto rot_mtx_x = glm::rotate(glm::mat4(1.0f), r_x, glm::vec3{ 1.0f, 0.0f, 0.0f });
	const auto rot_mtx_y = glm::rotate(glm::mat4(1.0f), r_y, glm::vec3{ 0.0f, 1.0f, 0.0f });
	const auto rot_mtx_z = glm::rotate(glm::mat4(1.0f), r_z, glm::vec3{ 0.0f, 0.0f, 1.0f });
	const auto modelview = rot_mtx_z * rot_mtx_x * rot_mtx_y;

	const auto viewport = fitting::get_opencv_viewport(image.cols, image.rows);
	const float aspect = static_cast<float>(image.cols) / image.rows;
	const auto ortho_projection = glm::ortho(-3.0f * aspect, 3.0f * aspect, -3.0f, 3.0f);
	const auto translate_topright = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, 0.65f, 0.0f));
	const auto o_2d = glm::project(origin, modelview, translate_topright * ortho_projection, viewport);
	const auto x_2d = glm::project(x_axis, modelview, translate_topright * ortho_projection, viewport);
	const auto y_2d = glm::project(y_axis, modelview, translate_topright * ortho_projection, viewport);
	const auto z_2d = glm::project(z_axis, modelview, translate_topright * ortho_projection, viewport);
	cv::line(image, cv::Point2f{ o_2d.x, o_2d.y }, cv::Point2f{ x_2d.x, x_2d.y }, { 0, 0, 255 });
	cv::line(image, cv::Point2f{ o_2d.x, o_2d.y }, cv::Point2f{ y_2d.x, y_2d.y }, { 0, 255, 0 });
	cv::line(image, cv::Point2f{ o_2d.x, o_2d.y }, cv::Point2f{ z_2d.x, z_2d.y }, { 255, 0, 0 });
};
