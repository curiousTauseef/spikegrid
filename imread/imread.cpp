#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdint.h>
#include "imread.h"
#include "../sizes.h"
cv::Mat ReadImage()
{
    std::string path= "input_maps/test.png";
    cv::Mat m =cv::imread(path); //the pixels are stored with type cv::Vec3b - third element is red.  0 is black, 255 is white?
    return m;
}
cv::Mat imcache; //elements have type Vec3b
void ApplyStim(Compute_float* voltsin,const Compute_float timemillis)
{
    cv::Mat m = ReadImage();
   // std::cout << timemillis << std::endl;
    for (int x=0;x<grid_size;x++)
    {
        for (int y=0;y<grid_size;y++)
        {
            cv::Vec3b pixel = m.at<cv::Vec3b>(x,y);
     //       std::cout << pixel << std::endl;
            if (pixel == cv::Vec3b(0,0,0))
            {
                voltsin[x*grid_size+y]=-100;
            }
            else if ( abs(timemillis-80.0)<.01 && pixel == cv::Vec3b(0,0,255))
            {
                voltsin[x*grid_size+y]=100;
            }

        }
    }
}