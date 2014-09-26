/// \file
#include "output.h"
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "openCVAPI/api.h"
#ifdef OPENCV
void cvdispInit(const char** const names,const int count)
{
    for (int i=0;i<count;i++)
    {
        output_s out = getOutputByName(names[i]);
        cvNamedWindow(out.name,CV_WINDOW_NORMAL);
    }
}
void cvdisp (const char** const names, const int count)
{
    for (int i=0;i<count;i++)
    {
        output_s out = getOutputByName(names[i]);
        const unsigned int size = (out.data.TA_data.size - (2*out.data.TA_data.offset))*out.data.TA_data.subgrid;
        Compute_float* data = taggedarrayTocomputearray(out.data.TA_data);
        PlotColored(out.name,data,out.data.TA_data.minval,out.data.TA_data.maxval,size);
        cvWaitKey(1);
        free(data);
    }
}
#endif
