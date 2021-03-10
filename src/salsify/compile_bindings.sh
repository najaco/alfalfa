#!/bin/bash
g++ -DHAVE_CONFIG_H  -I. -I../..  -I./../util -I./../decoder -I./../display -I./../input -I./../encoder -I./../net -fPIC $(python3 -m pybind11 --includes)  -std=c++14 -pthread  -pedantic -O3 -Wall -shared in_time_encoder.cc -o in_time_encoder$(python3-config --extension-suffix) ../net/libnet.a ../encoder/libalfalfaencoder.a ../input/libalfalfainput.a ../decoder/libalfalfadecoder.a ../util/libalfalfautil.a -lx264  -ljpeg $(python3-config --ldflags) 

# g++ -DHAVE_CONFIG_H  -I. -I../..  -I./../util -I./../decoder -I./../display -I./../input -I./../encoder -I./../net -fPIC $(python3 -m pybind11 --includes)  -std=c++14 -pthread  -pedantic -Wall -Wextra -Weffc++ -Werror -DNDEBUG -g -O2 -MT in_time_encoder.o -MD -MP -MF .deps/in_time_encoder.Tpo -c -o in_time_encoder.o in_time_encoder.cc

# in_time_encoder_SOURCES = in_time_encoder.cc
# in_time_encoder_CXXFLAGS = -O3 -Wall -shared -std=c++11 -fPIC
# in_time_encoder_CPPFLAGS = $(python3 -m pybind11 --includes) $(AM_CPPFLAGS)
# in_time_encoder_LDADD = ../net/libnet.a ../encoder/libalfalfaencoder.a $(BASE_LDADD)
# in_time_encoder_LDFLAGS = -pthread

