/*
  File: img_trans_client.cpp
  Date: April, 2018
  Author: Dagim Sisay <dagiopia@gmail.com>
  License: AGPL
*/

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#include <grpcpp/grpcpp.h>

#include "cv_img.grpc.pb.h"

#define SHOW_IMAGE 0
#define FACE_LOCATION 1
#define RED_LOCATION 2

class ImageTransferClient {
 public:
  ImageTransferClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(cv_img::img_trans_srv::NewStub(channel)) {}

  std::string send_img(cv::Mat in, int op) {
    cv_img::img_d img;

    std::vector<unsigned char> buff;
    unsigned char *bufa;
    cv::imencode(".jpg", in, buff);
    size_t d_size = buff.size();
    bufa = new unsigned char[d_size];
    for(int i = 0; i < d_size; i++)
    	bufa[i] = buff[i];

    img.set_img_data(bufa, d_size);
    img.set_operation(op);

    cv_img::ret s;
    cv_img::rect r;
    cv_img::circ c;

    grpc::ClientContext ctxt;
    grpc::Status status;

    if(SHOW_IMAGE == op)
      status = stub_->img_trans(&ctxt, img, &s);
    else if(FACE_LOCATION == op)
      status = stub_->img_trans(&ctxt, img, &r);
    else if(RED_LOCATION == op)
      status = stub_->img_trans(&ctxt, img, &c);

    
    std::string srep = "";
    if (status.ok()) {
      switch (op) {
        case SHOW_IMAGE:
	  srep = s.reply(); break;
	case FACE_LOCATION:
	  srep = "X: " + std::to_string(r.x()) +
	         "Y: " + std::to_string(r.y()) +
		 "W: " + std::to_string(r.w()) +
		 "H: " + std::to_string(r.h());
	  break;
	case RED_LOCATION:
	  srep = "CX: " + std::to_string(r.cx()) +
	         "CY: " + std::to_string(r.cy()) +
		 "R: " + std::to_string(r.r());
      }
      return srep;
    } 
    else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<cv_img::img_trans_srv::Stub> stub_;
};

int main(int argc, char** argv) {
  
  int op = 0;
  if ( 1 < argc)
    op = (int)strtol(argv[1], NULL, 10);
  if (0 > op && 2 < op)
    op = 0;

  cv::VideoCapture cap(0);
  if(!cap.isOpened())
  { std::cout<<"Couldn't Open Camera\n"; return -1; }
  
  cv::Mat frame;
  cap >> frame;
  if(frame.empty())
  { std::cout<<"Couldn't Capture Image\n"; return -1; }

  ImageTransferClient img_trns(grpc::CreateChannel(
      "localhost:7070", grpc::InsecureChannelCredentials()));
  
  std::string reply = img_trns.send_img(frame, op);
  std::cout << "Return: " << reply << std::endl;

  return 0;
}
