/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/* Copyright 2013-2018 the Alfalfa authors
                       and the Massachusetts Institute of Technology

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

      1. Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <getopt.h>

#include <cstdlib>
#include <random>
#include <unordered_map>
#include <utility>
#include <tuple>
#include <queue>
#include <deque>
#include <thread>
#include <condition_variable>
#include <future>

#include "socket.hh"
#include "packet.hh"
#include "poller.hh"
#include "optional.hh"
#include "player.hh"
#include "display.hh"
#include "paranoid.hh"
#include "procinfo.hh"

#if __has_include("pybind11/pybind11.h")
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#endif

using namespace std;
using namespace std::chrono;
using namespace PollerShortNames;

class DecoderPlayer
{
  queue<RasterHandle> display_queue;
  mutex mtx;
  condition_variable cv;

private:
  void display_task(const VP8Raster &example_raster, bool fullscreen)
  {
    VideoDisplay display{example_raster, fullscreen};

    while (true)
    {
      unique_lock<mutex> lock(mtx);
      cv.wait(lock, []() { return not display_queue.empty(); });

      while (not display_queue.empty())
      {
        display.draw(display_queue.front());
        display_queue.pop();
      }
    }
  }

  void enqueue_frame(FramePlayer &player, const Chunk &frame)
  {
    // static int frame_no = 0;
    if (frame.size() == 0)
    {
      return;
    }

    //cerr << "Size of frame is " << frame.size()  << " bytes or " << frame.size() * 8 << " bits" << endl;
    cerr << frame.size() << endl;
    const Optional<RasterHandle> raster = player.decode(frame);

    async(launch::async,
          [&raster]() {
            if (raster.initialized())
            {
              lock_guard<mutex> lock(mtx);
              display_queue.push(raster.get());
              cv.notify_all();
            }
          });
  }

  void run()
  {

    /* construct FramePlayer */
    FramePlayer player(paranoid::stoul(argv[optind + 1]), paranoid::stoul(argv[optind + 2]));
    player.set_error_concealment(true);

    /* construct display thread */
    thread([&player, fullscreen]() { display_task(player.example_raster(), fullscreen); }).detach();

    /* decoder states */
    uint32_t current_state = player.current_decoder().get_hash().hash();
    const uint32_t initial_state = current_state;
    deque<uint32_t> complete_states;
    unordered_map<uint32_t, Decoder> decoders{{current_state, player.current_decoder()}};
  }

}

#if __has_include("pybind11/pybind11.h")
PYBIND11_MODULE(decoder_player, m)
{
  m.doc() = "pybind11 bindings for Decoder Player";
  pybind11::class_<FramePlayer>(m, "FramePlayer")
      .def(pybind11::init<const uint16_t, const uint16_t>())
      .def("set_error_concealment", &FramePlayer::set_error_concealment);
  pybind11::class_<Chunk>(m, "Chunk")
      .def(pybind11::init<const uint8_t *, const uint64_t &>());
}
#endif