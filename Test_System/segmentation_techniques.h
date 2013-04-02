#ifndef SEGMENTATION_TECHNIQUES_H
#define SEGMENTATION_TECHNIQUES_H


#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

class Segmentation_techniques
{
public:
    Segmentation_techniques();
};


cv::Mat Global_Sobel(cv::Mat input_img,int kernel_size,int histogram_percentile, int dx,int dy,bool Otsu);
cv::Mat Local_Sobel(cv::Mat input_img,int sub_images,int kernel_size,int histogram_percentile, int dx,int dy,bool Otsu);

cv::Mat Global_Scharr(cv::Mat input_img,int histogram_percentile, bool dx_checked,bool dy_checked,bool Otsu);
cv::Mat Local_Scharr(cv::Mat input_img,int sub_images,int histogram_percentile, bool dx_checked,bool dy_checked,bool Otsu);

cv::Mat Histogram_seg(int desired_percentage,cv::Mat edge_img,cv::Mat original_img);
cv::Mat Histogram_seg(int desired_percentage,cv::Mat edge_img);

cv::Mat Otsu_on_Histo(cv::Mat histogram, cv::Mat image);

#endif // SEGMENTATION_TECHNIQUES_H
