#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include "cv.h"
#include "highgui.h"

// initialize global variables
const int maxjobs = 1000;
const int rows = 10;
const int cols = 10;
char* winname = "viewer0";
char** dirnames;
int upto=0;
void mousecb(int event, int x,int y,int dummy,void* dummy2) 
{
    if (event==CV_EVENT_LBUTTONDOWN)
    {
        int idx = (x/100)*cols+(y/100);
        printf("%s\n",dirnames[idx+upto]);
    }
}
int compare(const void* a, const void* b)
{
    const char * ca = *(const char**)a;
    const char * cb = *(const char**)b;
    int ia = atoi(ca+4);
    int ib = atoi(cb+4);
    return ia-ib;
}
int main() {
    //scan directory
    dirnames = malloc(sizeof(char*)*maxjobs);
    int dirno=0;
    DIR* d = opendir(".");
    struct dirent* dir;
    if (d)
    {
        while ((dir=readdir(d)) != NULL)
        {
            if (!strncmp(dir->d_name,"job-",4))
            {
                dirnames[dirno]=malloc(strlen(dir->d_name));
                strcpy(dirnames[dirno],dir->d_name);
                dirno++;
            }
        }
    }
    qsort(dirnames,dirno,sizeof(char*),compare);
    //set up video inputs
    cvNamedWindow(winname,CV_WINDOW_AUTOSIZE);
    cvMoveWindow(winname,0,0);
    cvSetMouseCallback(winname,mousecb,0);
    int size;
    CvCapture* caps[rows*cols];
    int vididx = 0;
    int iterations = 0;
    while (vididx < dirno)
    {
        int vcount=0;
        upto = vididx;
        for (int i=0;i<rows*cols && vididx< dirno ;i++)
        {
            char buf[100];
            sprintf(buf,"%s/test.avi",dirnames[vididx]);
            caps[i] = cvCreateFileCapture(buf);
            if (caps[i]){size = (int)cvGetCaptureProperty(caps[i],CV_CAP_PROP_FRAME_HEIGHT);}
            vididx++;
            vcount++;
        }
        if (size==0) {printf("No videos found\n");return(0);}
        IplImage* frame;
        IplImage* dispimage = cvCreateImage(cvSize(size*rows,size*cols),8,3);
        // display video frame by frame
        while(1) 
        {
            for (int i=0; i<vcount;i++)
            {
                frame=cvQueryFrame(caps[i]);
                if (!frame) 
                {
                    if (i==0) goto done;
                    else continue;
                }
                cvSetImageROI(dispimage,cvRect((i/cols)*size,(i%cols)*size,size,size));
                cvResize(frame,dispimage,CV_INTER_LINEAR);
                cvResetImageROI(dispimage);
            }
            cvShowImage(winname,dispimage);
                int c = cvWaitKey(33);
                if (c==27) break;
        }
done:
        // free memory
        for (int i=0; i < vcount;i++)
        {
            cvReleaseCapture( &caps[i] );
        }
        iterations++;
    }
    return(0);
}
