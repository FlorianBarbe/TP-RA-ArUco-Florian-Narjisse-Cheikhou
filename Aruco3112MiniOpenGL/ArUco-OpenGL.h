//
//  ArUco-OpenGL.h
//
//  Created by Jean-Marie Normand on 04/01/21.
//  Copyright (c) 2021 Centrale Nantes. All rights reserved.
//

#ifndef UserPerspectiveAR_ArUco_OpenGL_h
#define UserPerspectiveAR_ArUco_OpenGL_h

#include <Windows.h>

#ifdef __APPLE__
#include <OPENGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <iostream>

#include <fstream>
#include <sstream>

#include "ObjLoader.h"
#include "aruco.h" // ArUco 3.1.12 standalone library
#include <opencv2/opencv.hpp>

class ArUcoManager {
  // Attributes
protected:
  // OBJ model for 3D rendering
  Model_OBJ *m_Model;

  // Intrinsics file for the camera
  std::string m_IntrinsicFile;

  // Size of the marker (in meters)
  float m_MarkerSize;

  // The Marker Detector
  aruco::MarkerDetector m_PPDetector;

  // Vector of detected markers in the image
  std::vector<aruco::Marker> m_Markers;

  // OpenCV matrices storing the images
  // Input Image
  cv::Mat m_InputImage;

  // Undistorted image
  cv::Mat m_UndInputImage;

  // Resized image
  cv::Mat m_ResizedImage;

  // Camera parameters
  aruco::CameraParameters m_CameraParams;

  // Size of the OpenGL window size
  cv::Size m_GlWindowSize;

  // Methods
public:
  // Constructor
  ArUcoManager(std::string intrinFileName, float markerSize);
  // Destructor
  ~ArUcoManager();

  void drawBox(GLfloat size, GLenum type);

  // Detect marker and draw things
  void doWork(cv::Mat inputImg);

  // Draw axis function
  void drawAxis(float axisSize);

  // Draw axis function
  void drawWireCube(GLdouble size);

  // GLUT functionnalities

  // Drawing function
  void drawScene();

  // Idle function
  void idle(cv::Mat newImage);

  // Resize function
  void resize(GLsizei iWidth, GLsizei iHeight);
  void resizeCameraParams(cv::Size newSize);

  // Test using ArUco to display a 3D cube in OpenCV
  void draw3DCube(cv::Mat img, int markerInd = 0);
  void draw3DAxis(cv::Mat img, int markerInd = 0);
};

#endif
