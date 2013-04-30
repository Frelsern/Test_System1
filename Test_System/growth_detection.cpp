#include "growth_detection.h"

Growth_Detection::Growth_Detection()
{
}

//Finds percentage of foreground pixels
int percentage_foreground(cv::Mat input_image)
{
    int nc = input_image.cols*input_image.rows;
    int foreground = 0;

    uchar* data = input_image.ptr<uchar>(j);
    for(int i = 0; i<nc;i++)
    {
        if(data[i] == 1)
        {
           foreground = foreground+1;
        }
    }

    int percentage = foreground/nc;
    return percentage;
}

//for now it finds the percentage of growth on a scale from standard growth to 100% growth
int Growth_Detection_algo(int standard_percentage, cv::Mat input_image)
{
    int input_image_percentage = percentage_foreground(input_image);
    int diff = 0;

    if(input_image_percentage>standard_percentage)//dont accept negative growth
    {
        diff = standard_percentage-input_image_percentage;
        diff = diff*100/(100-standard_percentage);//rescaling difference to percentage of growth from standard growth to full growth

    }

    return diff;

}
