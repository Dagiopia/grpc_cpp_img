/*
  File: img_proc_server.cpp
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


class ImageTransferService final : public cv_img::img_trans_srv::Service {
  grpc::Status img_trans_s(grpc::ServerContext* ctxt, const cv_img::img_d *img, cv_img::ret *reply) 
    override {
    cv::Mat frame;
    size_t dlen = img->img_data().length();
    std::vector<unsigned char> dim(img->img_data().begin(), img->img_data().end());
    frame = cv::imdecode(dim, 1);
    std::string srep = "-";
    cv::imshow("img", frame);
    cv::waitKey(0);
    srep = "Done Showing";
    reply->set_reply(srep);
    return grpc::Status::OK;
  }

  grpc::Status img_trans_f(grpc::ServerContext *ctxt, const cv_img::img_d *img, cv_img::rects *recs)
    override {
      cv::Mat frame;
      size_t dlen = img->img_data().length();
      std::vector<unsigned char> dim(img->img_data().begin(), img->img_data().end());
      frame = cv::imdecode(dim, 1);
      cv::CascadeClassifier fdet;
      std::vector<cv::Rect> out;
      fdet.load("haarcascade_frontalface_alt.xml");
      fdet.detectMultiScale(frame, out, 1.1, 4, 
		      0 | cv::CASCADE_SCALE_IMAGE, 
		      cv::Size(30, 30));
      for(int i = 0; i < out.size(); i++){
	cv_img::rect *trec = recs->add_rcts();
	trec->set_x(out[i].x); trec->set_y(out[i].y);
        trec->set_w(out[i].width); trec->set_h(out[i].height);
      }
      return grpc::Status::OK;
  }

  grpc::Status img_trans_r(grpc::ServerContext *ctxt, const cv_img::img_d *img, cv_img::circ *cir)
    override {
      cv::Mat frame;
      size_t dlen = img->img_data().length();
      std::vector<unsigned char> dim(img->img_data().begin(), img->img_data().end());
      frame = cv::imdecode(dim, 1);
      std::vector<std::vector<cv::Point> > cntrs;
      std::vector<cv::Vec4i> hier;
      std::vector<int> hsv_lower({174, 0, 0});
      std::vector<int> hsv_upper({179, 255, 255});
      cv::inRange(frame, hsv_lower, hsv_upper, frame);
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
        cir->set_cx(cent.x);
        cir->set_cy(cent.y);
        cir->set_r(rad);
      }
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
