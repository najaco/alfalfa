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

#include "yuv4mpeg.hh"
#include "encoder.hh"
#include "camera.hh"


using namespace std;

class InTimeEncoder
{
private:
  string camera_device;
  string pixel_format;
  atomic<size_t> target_size;
  queue<vector<uint8_t>> encoded_frames;
  mutex frames_mtx;
  void push_frame(const vector<uint8_t> &frame)
  {
    lock_guard<std::mutex> lock(frames_mtx);
    encoded_frames.push(frame);
  }

public:
  InTimeEncoder(const string &camera_device, const string &pixel_format, const size_t target_size) : camera_device(camera_device), pixel_format(pixel_format), target_size(target_size), encoded_frames(queue<vector<uint8_t>>()), frames_mtx() {}
  
  void setTargetSize(const size_t target_size)
  {
    this->target_size.store(target_size, std::memory_order_relaxed);
  }
  
  size_t getTargetSize()
  {
    return target_size.load(std::memory_order_relaxed);
  }
  
  vector<uint8_t> get_frame(bool pop_frame = false)
  {
    lock_guard<std::mutex> lock(frames_mtx);
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
  
  int run()
  {
    // Pacer pacer;
    vector<uint64_t> cumulative_fpf;
    if (not PIXEL_FORMAT_STRS.count(pixel_format))
    {
      throw runtime_error("unsupported pixel format");
    }
    /* camera device */
    Camera camera{1280, 720, PIXEL_FORMAT_STRS.at(this->pixel_format), this->camera_device};

    /* construct the encoder */
    Encoder base_encoder{camera.display_width(), camera.display_height(),
                         false /* two-pass */, REALTIME_QUALITY};

    /* encoded frame index */
    unsigned int frame_no = 0;

    /* latest raster that is received from the input */
    Optional<RasterHandle> last_raster;

    while (1)
    {

      last_raster = camera.get_next_frame();

      if (not last_raster.initialized())
      {
        return EXIT_FAILURE;
      }

      RasterHandle raster = last_raster.get();
      vector<uint8_t> output = base_encoder.encode_with_target_size(raster.get(), this->getTargetSize());
      this->push_frame(output);
      cerr << "Encoded Frame " << frame_no++ << " with size " << this->getTargetSize() << endl;
    }
    return EXIT_FAILURE;
  }
};

int main(){
  printf("Hello World\n");
  exit(0);
}