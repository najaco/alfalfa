// #include <getopt.h>

#include <cstdlib>
#include <iostream>
#include <chrono>
#include <vector>
#include <limits>
#include <thread>
#include <future>
#include <algorithm>
#include <queue>
#include <atomic>
#include <limits>
#include <semaphore.h>
#include <pthread.h> 
#include <string>

// #include "yuv4mpeg.hh"
#include "encoder.hh"
#include "camera.hh"
#if __has_include("pybind11/pybind11.h")
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#endif

using namespace std;

class InTimeEncoder
{
private:
  string camera_device;
  string pixel_format;
  queue<vector<uint8_t>> encoded_frames;
  queue<RasterHandle> frames;
  Camera camera;
  Encoder encoder;

public:
  InTimeEncoder(const string &camera_device, const string &pixel_format) : camera_device(camera_device), pixel_format(pixel_format), encoded_frames(queue<vector<uint8_t>>()),/* sema_encoded_frame_queue(), sema_frame_queue(), */camera(Camera(1280, 720, PIXEL_FORMAT_STRS.at(this->pixel_format), camera_device)), encoder(Encoder(camera.display_width(), camera.display_height(), false, REALTIME_QUALITY))
  {
  }

  vector<uint8_t> get_encoded_frame(bool pop_frame = false)
  {
    if (this->encoded_frames.empty())
    {
      return vector<uint8_t>(); // For now
    }
    auto frame = this->encoded_frames.front();
    if (pop_frame)
    {
      this->encoded_frames.pop();
    }
    return frame;
  }

  // int run()
  // {

  //   vector<uint64_t> cumulative_fpf;
  //   if (not PIXEL_FORMAT_STRS.count(pixel_format))
  //   {
  //     throw runtime_error("unsupported pixel format");
  //   }
  //   /* camera device */
  //   Camera camera{1280, 720, PIXEL_FORMAT_STRS.at(this->pixel_format), this->camera_device};

  //   /* construct the encoder */
  //   Encoder base_encoder{camera.display_width(), camera.display_height(),
  //                        false /* two-pass */, REALTIME_QUALITY};

  //   /* encoded frame index */
  //   unsigned int frame_no = 0;

  //   /* latest raster that is received from the input */
  //   Optional<RasterHandle> last_raster;
  //   cout << "Started camera/encoder run" << endl;
  //   while (1)
  //   {

  //     last_raster = camera.get_next_frame();
  //     cout << "captured frame " << frame_no++ << " from camera" << endl;
  //     if (not last_raster.initialized())
  //     {
  //       return EXIT_FAILURE;
  //     }

  //     RasterHandle raster = last_raster.get();
  //     vector<uint8_t> output = base_encoder.encode_with_target_size(raster.get(), this->getTargetSize());
  //     this->push_frame(output);
  //   }
  //   return EXIT_FAILURE;
  // }
  bool capture_frame(){
    auto raster = this->camera.get_next_frame();
    if(not raster.initialized()){
      return false;
    }
    this->frames.push(raster.get());
    return true;
  }
  bool encode_last_frame_with_target_size(size_t target_size){
    if(this->frames.size() <= 0){
      return false;
    }
    RasterHandle raster = this->frames.front();
    this->frames.pop();
    vector<uint8_t> output = this->encoder.encode_with_target_size(raster.get(), target_size);
    this->encoded_frames.push(move(output));
    return true;
  }
};

#if __has_include("pybind11/pybind11.h")
PYBIND11_MODULE(in_time_encoder, m)
{
  m.doc() = "pybind11 bindings for InTimeEncoder";
  pybind11::class_<InTimeEncoder>(m, "InTimeEncoder")
      .def(pybind11::init<const std::string &, const std::string &>())
      .def("get_encoded_frame", &InTimeEncoder::get_encoded_frame)
      .def("capture_frame", &InTimeEncoder::capture_frame)
      .def("encode_last_frame_with_target_size", &InTimeEncoder::encode_last_frame_with_target_size);
}
#endif

// int main(){
//   InTimeEncoder ite("/dev/video0", "MJPG", 100000);
//   vector<uint8_t> v{2, 4, 6, 8, 9};
//   ite.push_frame(v);
//   vector<uint8_t> v2 = ite.get_frame(true);
//   cout << "Program Terminated" << endl;
// }