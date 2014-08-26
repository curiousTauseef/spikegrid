#include <opencv2/contrib/contrib.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;
void PlotColored(const char* winname,const double* data,const double min,const double max,const int size)
{
	double* dispmat = (double*)malloc(sizeof(*dispmat)*size*size);
	for (int i=0;i<size;i++)
	{
		for (int j=0;j<size;j++)
		{
			dispmat[i*size+j]=(data[i*size+j] - min)/(max-min);
		}
	}
	Mat m(size,size,CV_64F,dispmat);
	Mat outmat;
	applyColorMap(m,outmat,COLORMAP_JET);
	imshow(winname,outmat);
	waitKey(0);
	free(dispmat);
}

