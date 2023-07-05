#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <time.h>

using namespace std::chrono;

double current_millis();

int _get_millis_from_tp(const system_clock::time_point &t);

std::tm _tm_from_tp(const system_clock::time_point &t);

std::string format_timepoint_iso8601(const system_clock::time_point &t,
                                     bool add_zone = true,
                                     bool add_millis = true);

void stamp_image(cv::Mat &image,
                 system_clock::time_point t = system_clock::now(),
                 float ypos = 0.2);
