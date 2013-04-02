#include "segmentation_techniques.h"
#include <QDebug>

Segmentation_techniques::Segmentation_techniques()
{
}


cv::Mat Global_Sobel(cv::Mat input_img,int kernel_size,int histogram_percentile, int dx,int dy,bool Otsu)
{
    cv::Mat output_img;
    if((input_img.channels()==1) & (!input_img.empty()) & (kernel_size > dx) & (kernel_size > dy) & (dx+dy>0))
    {
        cv::Mat edge_img;
        if((input_img.channels()==1) & (!input_img.empty()) & (kernel_size > dx) & (kernel_size > dy) & (dx+dy>0))
        {
            cv::Sobel(input_img,edge_img,CV_32F,dx,dy,kernel_size,1,0,cv::BORDER_REPLICATE);
            edge_img.convertTo(edge_img,CV_8U);

        }
        if(Otsu)
        {
            output_img = Histogram_seg(histogram_percentile,edge_img,input_img);
        }
        else
        {
            output_img = Histogram_seg(histogram_percentile,edge_img);
        }


        //binary_img.copyTo(output_img);


    }
    return output_img;
}

cv::Mat Local_Sobel(cv::Mat input_img,int sub_images,int kernel_size,int histogram_percentile, int dx, int dy,bool Otsu)
{
    cv::Mat output_img;
    if((input_img.channels()==1) & (!input_img.empty()) & (kernel_size > dx) & (kernel_size > dy) & (dx+dy>0))
    {
        //qDebug() << "valid";

        cv::Mat edge_img;
        input_img.convertTo(input_img,CV_32F);
        int colsize = input_img.cols;
        int rowsize = input_img.rows;

        cv::Mat sub_image,sub_image2;
        sub_image.create(rowsize/sub_images,colsize/sub_images,CV_32F);
        sub_image2.create(rowsize/sub_images,colsize/sub_images,CV_32F);
        input_img.copyTo(edge_img);
        //qDebug() << "sub image er: " << sub_image.cols << sub_image.rows << "antall sub images: " << Local_Sobel_numberofSubImages;
        int c,d,e,f;
        for(int a = 0;a<sub_images;a++)
        {
            if(a == 0)
            {
                c=0;
                d = floor(1.0*rowsize/sub_images)-1;
            }else
            {
                c = d+1;
                d = d+floor(1.0*rowsize/sub_images);
            }

            for(int b = 0;b<sub_images;b++)
            {
                if(b == 0)
                {
                    e=0;
                    f = floor(1.0*colsize/sub_images)-1;
                }else
                {
                    e = f+1;
                    f = f+floor(1.0*colsize/sub_images);
                }

                sub_image = edge_img(cv::Rect(e,c,sub_image.cols,sub_image.rows));
                sub_image.copyTo(sub_image2);

                    cv::Sobel(sub_image2,sub_image,CV_32F,dx,dy,kernel_size,1,0,cv::BORDER_REPLICATE);

                }
        }
        edge_img.convertTo(edge_img,CV_8U);
        //edge_img = Histogram_seg(histogram_percentile,edge_img,input_img);

        if(Otsu)
        {
            output_img = Histogram_seg(histogram_percentile,edge_img,input_img);
        }
        else
        {
            output_img = Histogram_seg(histogram_percentile,edge_img);
        }
    }




    return output_img;
}

cv::Mat Global_Scharr(cv::Mat input_img,int histogram_percentile, bool dx_checked,bool dy_checked,bool Otsu)
{
    cv::Mat output_img;
    //Segmenting the image
    if((input_img.channels()==1) & (!input_img.empty()))
    {
        cv::Mat edge_img;
        if(dx_checked & dy_checked)
        {
            cv::Mat seg_dx,seg_dy;
            cv::Mat abs_seg_x,abs_seg_y;
            cv::Sobel(input_img,seg_dx,CV_32F,1,0,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
            cv::Sobel(input_img,seg_dy,CV_32F,0,1,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
            cv::convertScaleAbs(seg_dx,abs_seg_x);
            cv::convertScaleAbs(seg_dy,abs_seg_y);
            cv::addWeighted(abs_seg_x,0.5,abs_seg_y,0.5,0,edge_img);
        }
        else if(dx_checked)
        {
           cv::Sobel(input_img,edge_img,CV_32F,1,0,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
           edge_img.convertTo(edge_img,CV_8U);
        }
        else if(dy_checked)
        {
            cv::Sobel(input_img,edge_img,CV_32F,0,1,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
            edge_img.convertTo(edge_img,CV_8U);
        }
        //output_img = Histogram_seg(histogram_percentile,output_img,input_img);
        if(Otsu)
        {
            output_img = Histogram_seg(histogram_percentile,edge_img,input_img);
        }
        else
        {
            output_img = Histogram_seg(histogram_percentile,edge_img);
        }
    }


    return output_img;
}

cv::Mat Local_Scharr(cv::Mat input_img,int sub_images,int histogram_percentile, bool dx_checked,bool dy_checked,bool Otsu)
{
    cv::Mat output_img;
    if((input_img.channels()==1) & (!input_img.empty()))
    {
        int colsize = input_img.cols;
        int rowsize = input_img.rows;

        cv::Mat sub_image,sub_image2,edge_img;

         edge_img = input_img.clone();
         if(!(dx_checked & dy_checked))
         {
             edge_img.convertTo(edge_img,CV_32F);
             sub_image.create(rowsize/sub_images,colsize/sub_images,CV_32F);
             sub_image2.create(rowsize/sub_images,colsize/sub_images,CV_32F);
         }
         else
         {
             sub_image.create(rowsize/sub_images,colsize/sub_images,CV_8U);
             sub_image2.create(rowsize/sub_images,colsize/sub_images,CV_8U);
         }


         int c,d,e,f;
         for(int a = 0;a<sub_images;a++)
         {
             if(a == 0)
             {
                 c=0;
                 d = floor(1.0*rowsize/sub_images)-1;
             }else
             {
                 c = d+1;
                 d = d+floor(1.0*rowsize/sub_images);
             }

             for(int b = 0;b<sub_images;b++)
             {
                 if(b == 0)
                 {
                     e=0;
                     f = floor(1.0*colsize/sub_images)-1;
                 }else
                 {
                     e = f+1;
                     f = f+floor(1.0*colsize/sub_images);
                 }
                 sub_image = edge_img(cv::Rect(e,c,sub_image.cols,sub_image.rows));
                 sub_image.copyTo(sub_image2);

                 if(dx_checked & dy_checked)
                 {
                     cv::Mat seg_dx,seg_dy;
                     cv::Mat abs_seg_x,abs_seg_y;
                     cv::Sobel(sub_image2,seg_dx,CV_32F,1,0,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
                     cv::Sobel(sub_image2,seg_dy,CV_32F,0,1,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
                     cv::convertScaleAbs(seg_dx,abs_seg_x);
                     cv::convertScaleAbs(seg_dy,abs_seg_y);
                     cv::addWeighted(abs_seg_x,0.5,abs_seg_y,0.5,0,sub_image);//not perfect
                 }
                 else if(dx_checked)
                 {
                    cv::Sobel(sub_image2,sub_image,CV_32F,1,0,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
                 }
                 else if(dy_checked)
                 {
                     cv::Sobel(sub_image2,sub_image,CV_32F,0,1,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
                 }

             }
         }
         if(!(dx_checked & dy_checked))
         {
             edge_img.convertTo(output_img,CV_8U);
         }

         if(Otsu)
         {
             output_img = Histogram_seg(histogram_percentile,edge_img,input_img);
         }
         else
         {
             output_img = Histogram_seg(histogram_percentile,edge_img);
         }
    }



    //output_img = Histogram_seg(histogram_percentile,output_img,input_img);

    return output_img;

}

cv::Mat Histogram_seg(int desired_percentage,cv::Mat edge_img,cv::Mat original_img)
{
    cv::Mat hist,output_img,original_image;
    original_img.copyTo(original_image);
    int histSize = 256;
    float ranges[] = {0,256};
    const float* histRange = {ranges};
    bool uniform = true;
    bool accumulate = false;

    cv::calcHist(&edge_img,1,0,cv::Mat(),hist,1,&histSize,&histRange,uniform,accumulate);

 /*   int hist_w = 512; int hist_h = 400;
    int bin_w = cvRound((double)hist_w/histSize);
    cv::Mat histImage(hist_h,hist_w,CV_8U,cv::Scalar(0,0,0));

    //Normalizing the result to [0, histImage.rows]
    cv::normalize(hist,hist,0,histImage.rows,cv::NORM_MINMAX,-1,cv::Mat() );

    //draw for each channel
    for (int i = 1;i<histSize;i++)
    {
        cv::line(histImage,cv::Point(bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ),
                 cv::Point( bin_w*(i), hist_h-cvRound(hist.at<float>(i)) ),
                 cv::Scalar(255,0,0),2,8,0 );
    }//kan droppe den koden her? trenger vel ikke å tege opp histogrammet, inegn som skal se det
    */


    //percentile2i from Digital image processing using matlab-----------------------//
    float sum = 0;
    float* data = hist.ptr<float>(0);
    for (int i = 0;i<256;i++)
    {
        sum = sum+data[i];
    }

    for(int i = 0;i<256;i++)
    {
        data[i] = data[i]/sum;
    }

     //cumulative sum
    for(int i = 1;i<256;i++ )
    {
        data[i] = data[i]+data[i-1];
    }

    //finding the index where the cummulative sum is equal to or greather than the changeable variabel
    float changeable_variable = (float)desired_percentage/100;
    float idx = 0;
    for (int i = 0;i<256;i++)
    {
        //if(hist.at<float>(i) >= changeable_variable)
        if(data[i] >= changeable_variable)
        {
            idx = i;
            break;
        }
    }

    //-----------------------------------------------------------------------------//

    /* //thresholding image
        int nl = edge_img.rows;
        int nc = edge_img.cols;
        if(edge_img.isContinuous())
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* edge_data = edge_img.ptr<uchar>(j);
            uchar* img_data = original_image.ptr<uchar>(j);

            for(int i = 0; i<nc;i++)
            {
                if(img_data[i]==255)
                {
                    img_data[i] = 254;//setting the value into range
                }

                if(edge_data[i] <= idx)
                {
                    img_data[i] = 255;//setting the value out of range
                }
            }
        }

        original_image.copyTo(output_img);
       cv::threshold(original_image, output_img, 0,255,cv::THRESH_BINARY | cv::THRESH_OTSU);//prøver med 254

*/


    //thresholding image
            int nl = edge_img.rows;
            int nc = edge_img.cols;
            if(edge_img.isContinuous())
            {
                nc = nc*nl;
                nl = 1; //1D array;
            }

            //loop exectued only once if the image is continious
            for(int j = 0; j<nl;j++)
            {
                uchar* edge_data = edge_img.ptr<uchar>(j);
                uchar* img_data = original_image.ptr<uchar>(j);

                for(int i = 0; i<nc;i++)
                {
                    if(edge_data[i] <= idx)
                    {
                        img_data[i] = 0;//setting the value out of range
                    }
                }
            }


    cv::Mat final_hist;
    cv::calcHist(&original_image,1,0,cv::Mat(),final_hist,1,&histSize,&histRange,uniform,accumulate);

      //  int hist_w = 512; int hist_h = 400;
      //  int bin_w = cvRound((double)hist_w/histSize);

    output_img = Otsu_on_Histo(final_hist, original_image);

   // original_image.copyTo(output_img);
    return output_img;//change after finisheed

}

cv::Mat Histogram_seg(int desired_percentage,cv::Mat edge_img)
{
    cv::Mat hist;
    int histSize = 256;
    float ranges[] = {0,256};
    const float* histRange = {ranges};
    bool uniform = true;
    bool accumulate = false;

    cv::calcHist(&edge_img,1,0,cv::Mat(),hist,1,&histSize,&histRange,uniform,accumulate);

 /*   int hist_w = 512; int hist_h = 400;
    int bin_w = cvRound((double)hist_w/histSize);
    cv::Mat histImage(hist_h,hist_w,CV_8U,cv::Scalar(0,0,0));

    //Normalizing the result to [0, histImage.rows]
    cv::normalize(hist,hist,0,histImage.rows,cv::NORM_MINMAX,-1,cv::Mat() );

    //draw for each channel
    for (int i = 1;i<histSize;i++)
    {
        cv::line(histImage,cv::Point(bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ),
                 cv::Point( bin_w*(i), hist_h-cvRound(hist.at<float>(i)) ),
                 cv::Scalar(255,0,0),2,8,0 );
    }//kan droppe den koden her? trenger vel ikke å tege opp histogrammet, inegn som skal se det
    */


    //percentile2i from Digital image processing using matlab-----------------------//
    float sum = 0;
    float* data = hist.ptr<float>(0);
    for (int i = 0;i<256;i++)
    {
        sum = sum+data[i];
    }

    for(int i = 0;i<256;i++)
    {
        data[i] = data[i]/sum;
    }

     //cumulative sum
    for(int i = 1;i<256;i++ )
    {
        data[i] = data[i]+data[i-1];
    }

    //finding the index where the cummulative sum is equal to or greather than the changeable variabel
    float changeable_variable = (float)desired_percentage/100;
    float idx = 0;
    for (int i = 0;i<256;i++)
    {
        //if(hist.at<float>(i) >= changeable_variable)
        if(data[i] >= changeable_variable)
        {
            idx = i;
            break;
        }
    }

    //-----------------------------------------------------------------------------//

    //thresholding image
    int nl = edge_img.rows;
    int nc = edge_img.cols;
    if(edge_img.isContinuous())
    {
        nc = nc*nl;
        nl = 1; //1D array;
    }

    //loop exectued only once if the image is continious
    for(int j = 0; j<nl;j++)
    {
        uchar* data = edge_img.ptr<uchar>(j);

        for(int i = 0; i<nc;i++)
        {
            if(data[i] > idx)
            {
                data[i] = 255;
            }
            else
            {
                data[i] = 0;
            }


        }
    }

    return edge_img;

}


cv::Mat Otsu_on_Histo(cv::Mat histogram, cv::Mat image)
{
    //this part of code is based on code found at
    //https://snipt.net/spechard/otsu-thresholding-with-opencv/
    //01.04.2013
    int hist_lvls = 256;
    float sum = 0;
    int total = 0;
    uchar* hist_data = histogram.ptr<uchar>(0);

    hist_data[0] = 0;//the histogram is dominated by 0s, they should however not count for the segmentation


    for(int i = 0;i<hist_lvls;i++)
    {
        sum = sum + i*hist_data[i];
        total = total+hist_data[i];//number of pixels
    }

    float sumB = 0;
    int wB = 0;
    int wF = 0;
    float varMax = 0;
    int threshold = 0;

    for(int i = 0;i<hist_lvls;i++)
    {
        wB += hist_data[i]; //weight background
        if(wB == 0)
        {
            continue;
        }

        wF = total-wB;
        if(wF == 0)
        {
            break;
        }

        sumB += (float)(i*hist_data[i]);

        float mB = sumB/wB;
        float mF = (sum-sumB)/wF;

        //calculating between class variance
        float varBetween = (float)wB*(float)wF*(mB-mF)*(mB-mF);
       // qDebug() << "between" <<varBetween;

        //Check if it is a new maximum
        if(varBetween>varMax)
        {
            varMax = varBetween;
            threshold = i;
        }

    }

    int nl = image.rows;
    int nc = image.cols;
    if(image.isContinuous())
    {
        nc = nc*nl;
        nl = 1; //1D array;
    }

    //loop exectued only once if the image is continious
    for(int j = 0; j<nl;j++)
    {
        uchar* img_data = image.ptr<uchar>(j);


        for(int i = 0; i<nc;i++)
        {
            if(img_data[i] > threshold)
            {
                img_data[i] = 255;//foreground
            }
            else
            {
                img_data[i]=0;//background
            }
        }
    }


    return image;

}

