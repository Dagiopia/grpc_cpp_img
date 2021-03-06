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

#define IMG_ENCODING ".jpg"

class ImageTransferClient {
 public:
  ImageTransferClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(cv_img::img_trans_srv::NewStub(channel)) {}

  std::string send_img(cv::Mat in, int op) {
    cv_img::img_d img;

    std::vector<unsigned char> buff;
    cv::imencode(IMG_ENCODING, in, buff);
    size_t d_size = buff.size();
    //for(int i = 0; i < d_size; i++)
    // 	bufa[i] = buff[i];

    unsigned char *bufa = &buff[0];
    
    img.set_img_data(bufa, d_size);
    img.set_encoding(IMG_ENCODING);

    cv_img::ret s;
    cv_img::rects r;
    cv_img::circ c;

    grpc::ClientContext ctxt;
    grpc::Status status;

    if(SHOW_IMAGE == op)
      status = stub_->img_trans_s(&ctxt, img, &s);
    else if(FACE_LOCATION == op)
      status = stub_->img_trans_f(&ctxt, img, &r);
    else if(RED_LOCATION == op)
      status = stub_->img_trans_r(&ctxt, img, &c);

    
    std::string srep = "";
    if (status.ok()) {
      switch (op) {
        case SHOW_IMAGE:
	  srep = s.reply(); break;
	case FACE_LOCATION:
	  for(int i = 0; i < r.rcts_size(); i++) { 
	      srep += "X: " + std::to_string(r.rcts(i).x()) +
	         "Y: " + std::to_string(r.rcts(i).y()) +
		 "W: " + std::to_string(r.rcts(i).w()) +
		 "H: " + std::to_string(r.rcts(i).h()) + "\n";
	  }
	  break;
	case RED_LOCATION:
	  srep = "CX: " + std::to_string(c.cx()) +
	         "CY: " + std::to_string(c.cy()) +
		 "R: " + std::to_string(c.r());
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
  else{
    std::cout<<argv[0]<<" OPERATION_CODE\n \t\t0 = SHOW_IMAGE\n \t\t"
	     <<"1 = FACE_LOCATION\n \t\t2 = RED_LOCATION\n";
    return 0;
  }
  if (0 > op || 2 < op)
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

  return 0;}
