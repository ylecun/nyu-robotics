using namespace std;

#include <cv.h>
#include <highgui.h>
#include <cvaux.h>

using namespace cv;

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <libusb.h>
#include "libfreenect.h"

#include <pthread.h>

Mat depthMat(Size(640,480),CV_16UC1),
    rgbMat(Size(640,480),CV_8UC3,Scalar(0));
pthread_t fnkt_thread;
freenect_device *f_dev;
pthread_mutex_t buf_mutex = PTHREAD_MUTEX_INITIALIZER;
freenect_context *f_ctx;
pthread_cond_t frame_cond = PTHREAD_COND_INITIALIZER;
 
int main(int argc, char **argv)
{
    int res;
 
    g_argc = argc;
    g_argv = argv;
 
    if (freenect_init(&f_ctx, NULL) < 0) {
        printf("freenect_init() failed\n");
        return 1;
    }
 
    freenect_set_log_level(f_ctx, FREENECT_LOG_INFO);
 
    int nr_devices = freenect_num_devices (f_ctx);
    printf ("Number of devices found: %d\n", nr_devices);
 
    int user_device_number = 0;
    if (argc > 1)
        user_device_number = atoi(argv[1]);
 
    if (nr_devices < 1)
        return 1;
 
    if (freenect_open_device(f_ctx, &f_dev, user_device_number) < 0) {
        printf("Could not open device\n");
        return 1;
    }
 
    freenect_set_tilt_degs(f_dev,freenect_angle);
    freenect_set_led(f_dev,LED_RED);
    freenect_set_depth_callback(f_dev, depth_cb);
    freenect_set_rgb_callback(f_dev, rgb_cb);
    freenect_set_rgb_format(f_dev, FREENECT_FORMAT_RGB);
    freenect_set_depth_format(f_dev, FREENECT_FORMAT_11_BIT);
 
    freenect_start_depth(f_dev);
    freenect_start_rgb(f_dev);
 
    res = pthread_create(&fnkt_thread, NULL, freenect_threadfunc, NULL);
    if (res) {
        printf("pthread_create failed\n");
        return 1;
    }
 
    while (!die) {
        fr++;
 
        imshow("rgb", rgbMat);
        depthMat.convertTo(depthf, CV_8UC1, 255.0/2048.0);
        imshow("depth",depthf);         
 
                 char k = cvWaitKey(5);
                 if( k == 27 ) break;
         }
 
    printf("-- done!\n");
 
    destroyWindow("rgb");
    destroyWindow("depth");
 
    pthread_join(fnkt_thread, NULL);
    pthread_exit(NULL);
}


void *freenect_threadfunc(void* arg) {
    cout << "freenect thread"<<endl;
    while(!die && freenect_process_events(f_ctx) >= 0 ) {}
    cout << "freenect die"<<endl;
    return NULL;
}

void depth_cb(freenect_device *dev, freenect_depth *depth, uint32_t timestamp)
{
    pthread_mutex_lock(&buf_mutex);
 
    //copy to ocv buf...
    memcpy(depthMat.data, depth, FREENECT_DEPTH_SIZE);
 
    got_frames++;
    pthread_cond_signal(&frame_cond);
    pthread_mutex_unlock(&buf_mutex);
}
 
void rgb_cb(freenect_device *dev, freenect_pixel *rgb, uint32_t timestamp)
{
    pthread_mutex_lock(&buf_mutex);
    got_frames++;
    //copy to ocv_buf..
    memcpy(rgbMat.data, rgb, FREENECT_RGB_SIZE);
 
    pthread_cond_signal(&frame_cond);
    pthread_mutex_unlock(&buf_mutex);
}

