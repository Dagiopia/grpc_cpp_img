/*
  File: img_trans_server.cpp
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

class ImageTransferService final : public cv_img::img_trans_srv::Service {
  grpc::Status img_trans(grpc::ServerContext* ctxt, const cv_img::img_d *img, cv_img::ret *reply) override {
    cv::Mat frame;
    std::vector<unsigned char> dim;
    size_t dlen = img->img_data().length();
    int op = img->operation();
    for(int i = 0 ; i < dlen; i++)
    	dim.push_back((unsigned char)img->img_data()[i]);
    frame = cv::imdecode(dim, 1);
    std::string srep = "-";
    if (SHOW_IMAGE == op){
      cv::imshow("img", frame);
      cv::waitKey(0);
      srep = "Donw Showing"
    }
    else if (FACE_LOCATION == op) {
      cv::CascadeClassifier fdet;
      std::vector<cv::Rect> out;
      fdet.load("haarcascade_frontalface_alt.xml");
      fdet.detectMultiScale(frame, out, 1.1, 4, 0|CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));
      cv_img::rect r;
      r->set_x(out.x); r->set_y(out.y);
      r->set_w(out.w); r->set_h(out.h);
    }
    else {
      std::vector<std::vector<cv::Point> > cntrs;
      std::vector<cv::Vec4i> hier;
      cv::inRange(frame, [174, 0, 0], [179, 0, 0], frame);
      cv::findContours(frame, cntrs, hier, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
      if(cntrs.size() > 0){
        double max_area = 0;
	int max_idx;
	for(int i = 0; i < cntrs.size(); i++){
          if ( cv::contourArea(cntrs[i]) > max_area){
	    max_area = cv::contourArea(cntrs[i]);
	    max_idx = i;
	  }
	}
	cv::Point2f cent;
	float rad;
	cv::minEnclosingCircle(cntrs[max_idx], cent, rad);
	cv_img::circ c;
	c->set_cx(cent.x);
	c->set_cy(cent.y);
	c->set_r(rad);
      }
    }
    reply->set_reply(srep);
    return grpc::Status::OK;
  }
};


int main(int argc, char** argv) {
  std::string srvr_add("0.0.0.0:7070");
  ImageTransferService srv;
  grpc::ServerBuilder srvrb;
  srvrb.AddListeningPort(srvr_add, grpc::InsecureServerCredentials());
  srvrb.RegisterService(&srv);
  std::unique_ptr<grpc::Server> srvr(srvrb.BuildAndStart());
  std::cout<<"Server Listening on " << srvr_add << std::endl;
  srvr->Wait();

  return 0;
}
