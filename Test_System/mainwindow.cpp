#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "segmentation_techniques.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //setup window
    ui->setupUi(this);
    this->setWindowTitle(QString("System 1, v.0.2.5"));

    //hide the Segmentation boxes
    ui->Global_Sobel_box->hide();
    ui->Local_Sobel_box->hide();
    ui->Global_Scharr_box->hide();
    ui->Local_Scharr_box->hide();
    ui->Global_Otsu_box->hide();
    ui->Local_Otsu_box->hide();
    ui->Thresholding_box->hide();
    ui->Adaptive_Thresholding_box->hide();

    //initializing parameters.
    Global_Sobel_hist_percentile = 90;
    Global_Sobel_dx = 1;
    Global_Sobel_dy = 1;
    Global_Sobel_kernel_size = 3;
    Local_Sobel_numberofSubImages = 5;
    Local_Sobel_hist_percentile = 90;
    Local_Sobel_dx = 1;
    Local_Sobel_dy = 1;
    Local_Sobel_kernel_size = 3;
    Global_Scharr_hist_percentile = 90;
    Local_Scharr_numberofSubImages = 5;
    Local_Scharr_hist_percentile = 90;
    Local_Otsu_numberofSubImages = 5;
    Threshold = 90;

    Dilation_iterations = 1;
    Gaussian_kernel_size = 1;
    Adaptive_Thresholding_kernel_size = 3;

    cspace = COLOR_NONE;
    thresh_met = THRESH_NONE;
    mode = NO_MODE;

    //Opening webcam and call the other processes.
    capWebcam.open(0);

    if(capWebcam.isOpened() == false)
    {
        //ui->plainTextEdit->appendPlainText("error: Webcam not accessed succesfully");
        return;

    }

    tmrTimer = new QTimer(this);
    connect(tmrTimer, SIGNAL(timeout()),this,SLOT(processFrameAndUpdateGUI()));
    //tmrTimer->start(1.0/60); //lavprioritet funksjon, hvis systemet bruker lenger tid får det lov til å btuke lenger tid
    tmrTimer->start(100);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processFrameAndUpdateGUI()
{
    //Executed time measurment
    double duration;
    duration = static_cast<double>(cv::getTickCount());

    capWebcam.read(image);//skru på når ferdig med bilder
    if(image.empty()==true) return;

    //change color channel ordering
    cv::cvtColor(image,image,CV_BGR2RGB);//skru på når ferdig med bilder


    //Converting to the given color space
    switch(cspace)
    {
    case COLOR_NONE:
       break;
    case LUMINANCE:
        //kan være kjappere å bare skaffe Y komponenten, i forhold til ferdiglagd og custom XYZ
        custom_XYS_image();
        on_Y_clicked();
        break;
    case X:
        custom_XYS_image();
        on_x_clicked();
        break;
    case Y:
        custom_XYS_image();
        on_y_clicked();
        break;
    case RED:
        on_Red_clicked();
        break;
    case GREEN:
        on_Green_clicked();
        break;
    case BLUE:
        on_Blue_clicked();
        break;
    case X2:
        cv::cvtColor(image,XYZ2,CV_RGB2XYZ);
        on_x2_clicked();
        break;
    case Y2:
        cv::cvtColor(image,XYZ2,CV_RGB2XYZ);
        on_y2_clicked();
        break;
    case LUMINANCE2:
        cv::cvtColor(image,XYZ2,CV_RGB2XYZ);
        on_Y2_clicked();
        break;
    case LAB:
        cv::cvtColor(image,Lab_image,CV_RGB2Lab);
        break;
    case L:
        cv::cvtColor(image,Lab_image,CV_RGB2Lab);
        on_L_clicked();
        break;
    case A:
        cv::cvtColor(image,Lab_image,CV_RGB2Lab);
        on_a_clicked();
        break;
    case B:
        cv::cvtColor(image,Lab_image,CV_RGB2Lab);
        on_b_clicked();
        break;
    }

    //Smoothing image before thresholding
    if(ui->Gaussian_checkBox->isChecked() & !processed_image.empty())
    {
        cv::GaussianBlur(processed_image,processed_image,cv::Size(Gaussian_kernel_size,Gaussian_kernel_size),0,0,cv::BORDER_DEFAULT);
    }

    //thresholding
    switch(thresh_met)
    {
    case THRESH_NONE:
        break;
    case GLOBAL_SOBEL:
        //on_Global_Sobel_clicked();
        Segmented_image = Global_Sobel(processed_image,Global_Sobel_kernel_size,Global_Sobel_hist_percentile,
                                       Global_Sobel_dx,Global_Sobel_dy,ui->Otsu_in_edge_checkBox->isChecked());
        break;
    case LOCAL_SOBEL:
        //on_Local_Sobel_clicked();
        Segmented_image = Local_Sobel(processed_image,Local_Sobel_numberofSubImages, Local_Sobel_kernel_size,
                                      Local_Sobel_hist_percentile,Local_Sobel_dx,Local_Sobel_dy,ui->Otsu_in_edge_checkBox->isChecked());
        break;
    case GLOBAL_SCHARR:
        //on_Global_Scharr_clicked();
        Segmented_image = Global_Scharr(processed_image,Global_Scharr_hist_percentile,ui->Global_Scharr_dx_checkBox->isChecked(),
                                        ui->Global_Scharr_dy_checkBox->isChecked(),ui->Otsu_in_edge_checkBox->isChecked());
        break;
    case LOCAL_SCHARR:
        //on_Local_Scharr_clicked();
        Segmented_image = Local_Scharr(processed_image,Local_Scharr_numberofSubImages,Local_Scharr_hist_percentile,
                                       ui->Local_Scharr_dx_checkBox->isChecked(),ui->Local_Scharr_dy_checkBox->isChecked(),ui->Otsu_in_edge_checkBox->isChecked());
        break;
    case GLOBAL_OTSU:
        on_Global_Otsu_clicked();
        break;
    case LOCAL_OTSU:
        on_Local_Otsu_clicked();
        break;
    case THRESHOLDING:
        on_Thresholding_clicked();
        break;
    case ADAPTIVE_THRESHOLDING:
        on_Adaptive_Thresholding_clicked();
        break;
    }


    //operations to improve the segmentation result
    if(!Segmented_image.empty())
    {        
        if(ui->Inversion_checkbox->isChecked())
        {
            int nl = Segmented_image.rows;
            int nc = Segmented_image.cols;
            if(Segmented_image.isContinuous())
            {
                nc = nc*nl;
                nl = 1; //1D array;
            }

            //loop exectued only once if the image is continious
            for(int j = 0; j<nl;j++)
            {
                uchar* data = Segmented_image.ptr<uchar>(j);
                for(int i = 0; i<nc;i++)
                {
                    data[i] = 255-data[i];
                }
            }
        }
        if(ui->Dilation_checkBox->isChecked())
        {
            cv::Mat Seg_copy;
            Segmented_image.copyTo(Seg_copy);
            cv::dilate(Seg_copy,Segmented_image,cv::Mat(),cv::Point(-1,-1),Dilation_iterations,cv::BORDER_CONSTANT,cv::morphologyDefaultBorderValue());

        }
    }

    //if a mode is chosen display the modes image instead of the color space image
    switch(mode)
    {
    case NO_MODE:
        //putting the correct color space image and segmented image on display.
        if(ui->Lab->isChecked())
        {
            QImage color_space_image = QImage((const unsigned char*)(Lab_image.data),
                                Lab_image.cols,Lab_image.rows,QImage::Format_RGB888);
            ui->label->setPixmap(QPixmap::fromImage(color_space_image));
        }
        else if(processed_image.channels()==1)
        {
            QImage color_space_image = QImage((const unsigned char*)(processed_image.data),
                                 processed_image.cols,processed_image.rows,QImage::Format_Indexed8);
            ui->label->setPixmap(QPixmap::fromImage(color_space_image));
        }
        break;
    case HOLE_DETECTION:
        //knaksje ta å lage en ny label, hide processed og vise denne, kan bli fixa på knappetrykka
        Hole_Detection();
        if(!hole_detected_image.empty())
        {
            QImage hole_detection_image = QImage((const unsigned char*)(hole_detected_image.data),
                                 hole_detected_image.cols,hole_detected_image.rows,QImage::Format_Indexed8);
            ui->label->setPixmap(QPixmap::fromImage(hole_detection_image));
        }
        break;
    case GROWTH_DETECTION:
        break;

    }

    //Show the segmented image in the right label
    if(!Segmented_image.empty())
    {
        QImage segmented_image = QImage((const unsigned char*)(Segmented_image.data),
                                        Segmented_image.cols,Segmented_image.rows,QImage::Format_Indexed8 );
        ui->processed_image_label->setPixmap(QPixmap::fromImage(segmented_image));
    }

    //time measurment part
    duration = static_cast<double>(cv::getTickCount())-duration;
    duration /=cv::getTickFrequency();//elapsed time in ms
    ui->Total_time_spent->appendPlainText(QString::number(duration) + QString("s"));


}
void MainWindow::on_actionOpen_Image_triggered()
{

    QString fileName = QFileDialog::getOpenFileName(this,tr("Open Image"),".",tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));
    image = cv::imread(fileName.toLatin1().data()) ;//.toAscii().data()


    //change color channel ordering
    cv::cvtColor(image,image,CV_BGR2RGB);
    //  Qt image
   QImage img = QImage((const unsigned char*)(image.data),
                        image.cols,image.rows,QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(img));
    //resize the label
    //ui->label->resize(ui->label->pixmap()->size());*/

  /*  XYZ_image.create(image.size(), image.type());

    for(int i = 0;i< image.cols;i++)
        for(int j = 0; j<image.rows;j++)
        {
            XYZ_image.at<cv::Vec3b>(j,i)[0] =  0.607*image.at<cv::Vec3b>(j,i)[0]+0.174*image.at<cv::Vec3b>(j,i)[1]+0.200*image.at<cv::Vec3b>(j,i)[2];
            XYZ_image.at<cv::Vec3b>(j,i)[1] =  0.299*image.at<cv::Vec3b>(j,i)[0]+0.587*image.at<cv::Vec3b>(j,i)[1]+0.114*image.at<cv::Vec3b>(j,i)[2];
            XYZ_image.at<cv::Vec3b>(j,i)[2] =  0.066*image.at<cv::Vec3b>(j,i)[1]+1.111*image.at<cv::Vec3b>(j,i)[2];
        }

    cv::cvtColor(image,XYZ2,CV_RGB2XYZ);
    cv::cvtColor(image,Lab_image,CV_RGB2Lab);

    */

}


void MainWindow::on_Global_Sobel_clicked()
{
     thresh_met = GLOBAL_SOBEL;

    //hide the different group boxes and show the global sobel one.
    ui->Global_Sobel_box->show();
    ui->Local_Sobel_box->hide();
    ui->Global_Scharr_box->hide();
    ui->Local_Scharr_box->hide();
    ui->Global_Otsu_box->hide();
    ui->Local_Otsu_box->hide();
    ui->Thresholding_box->hide();
    ui->Adaptive_Thresholding_box->hide();


    /*//Segment the image, finding edges with sobel filters
    if((processed_image.channels()==1) & (!processed_image.empty()) & (Global_Sobel_kernel_size > Global_Sobel_dx) & (Global_Sobel_kernel_size > Global_Sobel_dy) & (Global_Sobel_dx+Global_Sobel_dy>0))
    {
        cv::Sobel(processed_image,Segmented_image,CV_32F,Global_Sobel_dx,Global_Sobel_dy,Global_Sobel_kernel_size,1,0,cv::BORDER_REPLICATE);
        Segmented_image.convertTo(Segmented_image,CV_8U);

    }

    Histogram_segmentation(Global_Sobel_hist_percentile);*/

}
void MainWindow::on_Local_Sobel_clicked()
{
    thresh_met = LOCAL_SOBEL;

    //hide the different group boxes and show the local sobel one.
    ui->Global_Sobel_box->hide();
    ui->Local_Sobel_box->show();
    ui->Global_Scharr_box->hide();
    ui->Local_Scharr_box->hide();
    ui->Global_Otsu_box->hide();
    ui->Local_Otsu_box->hide();
    ui->Thresholding_box->hide();
    ui->Adaptive_Thresholding_box->hide();

   /* if((processed_image.channels()==1) & (!processed_image.empty()) & (Local_Sobel_kernel_size > Local_Sobel_dx) & (Local_Sobel_kernel_size > Local_Sobel_dy) & (Local_Sobel_dx+Local_Sobel_dy>0))
    {
       // Segmented_image.create(processed_image.size(),CV_32F);
        Segmented_image = processed_image.clone();
        Segmented_image.convertTo(Segmented_image,CV_32F);
        int colsize = Segmented_image.cols;
        int rowsize = Segmented_image.rows;

        cv::Mat sub_image,sub_image2;
        sub_image.create(rowsize/Local_Sobel_numberofSubImages,colsize/Local_Sobel_numberofSubImages,CV_32F);
        sub_image2.create(rowsize/Local_Sobel_numberofSubImages,colsize/Local_Sobel_numberofSubImages,CV_32F);

        //qDebug() << "sub image er: " << sub_image.cols << sub_image.rows << "antall sub images: " << Local_Sobel_numberofSubImages;
        int c,d,e,f;
        for(int a = 0;a<Local_Sobel_numberofSubImages;a++)
        {
            if(a == 0)
            {
                c=0;
                d = floor(1.0*rowsize/Local_Sobel_numberofSubImages)-1;
            }else
            {
                c = d+1;
                d = d+floor(1.0*rowsize/Local_Sobel_numberofSubImages);
            }

            for(int b = 0;b<Local_Sobel_numberofSubImages;b++)
            {
                if(b == 0)
                {
                    e=0;
                    f = floor(1.0*colsize/Local_Sobel_numberofSubImages)-1;
                }else
                {
                    e = f+1;
                    f = f+floor(1.0*colsize/Local_Sobel_numberofSubImages);
                }

                sub_image = Segmented_image(cv::Rect(e,c,sub_image.cols,sub_image.rows));
                sub_image.copyTo(sub_image2);

                //converting to float and doing sobel operation
               // if((Local_Sobel_kernel_size > Local_Sobel_dx) & (Local_Sobel_kernel_size > Local_Sobel_dy) & (Local_Sobel_dx+Local_Sobel_dy>0))
               // {
                    cv::Sobel(sub_image2,sub_image,CV_32F,Local_Sobel_dx,Local_Sobel_dy,Local_Sobel_kernel_size,1,0,cv::BORDER_REPLICATE);//need float to do sobel??
               // }
                //else
                  //  break;

               // cv::Sobel(sub_image2,sub_image,CV_32F,1,1,3,1,0,cv::BORDER_REPLICATE);//need float to do sobel??
                //new attempt, seems better
                if((Local_Sobel_kernel_size > Local_Sobel_dx) & (Local_Sobel_kernel_size > Local_Sobel_dy))
                {
                    cv::Mat seg_dx,seg_dy;
                    cv::Mat abs_seg_x,abs_seg_y;

                    if(Local_Sobel_dx>0)
                    {
                        cv::Sobel(sub_image2_x,seg_dx,CV_32F,Local_Sobel_dx,0,Local_Sobel_kernel_size,1,0,cv::BORDER_REPLICATE);
                        cv::convertScaleAbs(seg_dx,abs_seg_x);
                    }
                    else
                    {
                        abs_seg_x = cv::Mat::zeros(sub_image.size(), CV_8U);
                    }

                    if(Local_Sobel_dy>0)
                    {
                      cv::Sobel(sub_image2_y,seg_dy,CV_32F,0,Local_Sobel_dy,Local_Sobel_kernel_size,1,0,cv::BORDER_REPLICATE);
                      cv::convertScaleAbs(seg_dy,abs_seg_y);
                    }
                    else
                    {
                        abs_seg_y = cv::Mat::zeros(sub_image.size(), CV_8U);
                    }

                    cv::addWeighted(abs_seg_x,0.5,abs_seg_y,0.5,0,sub_image);

                }
                }
        }
    }
    Segmented_image.convertTo(Segmented_image,CV_8U);
    Histogram_segmentation(Local_Sobel_hist_percentile);*/

}

void MainWindow::on_Global_Scharr_clicked()
{
    thresh_met = GLOBAL_SCHARR;

    //hide the different group boxes and show the global sobel one.
    ui->Global_Sobel_box->hide();
    ui->Local_Sobel_box->hide();
    ui->Global_Scharr_box->show();
    ui->Local_Scharr_box->hide();
    ui->Global_Otsu_box->hide();
    ui->Local_Otsu_box->hide();
    ui->Thresholding_box->hide();
    ui->Adaptive_Thresholding_box->hide();

 /*   //Segmenting the image
    if((processed_image.channels()==1) & (!processed_image.empty()))
    {
        if(ui->Global_Scharr_dx_checkBox->isChecked() & ui->Global_Scharr_dy_checkBox->isChecked())
        {
            cv::Mat seg_dx,seg_dy;
            cv::Mat abs_seg_x,abs_seg_y;
            cv::Sobel(processed_image,seg_dx,CV_32F,1,0,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
            cv::Sobel(processed_image,seg_dy,CV_32F,0,1,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
            cv::convertScaleAbs(seg_dx,abs_seg_x);
            cv::convertScaleAbs(seg_dy,abs_seg_y);
            cv::addWeighted(abs_seg_x,0.5,abs_seg_y,0.5,0,Segmented_image);
        }
        else if(ui->Global_Scharr_dx_checkBox->isChecked())
        {
           cv::Sobel(processed_image,Segmented_image,CV_32F,1,0,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
           Segmented_image.convertTo(Segmented_image,CV_8U);
        }
        else if(ui->Global_Scharr_dy_checkBox->isChecked())
        {
            cv::Sobel(processed_image,Segmented_image,CV_32F,0,1,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
            Segmented_image.convertTo(Segmented_image,CV_8U);
        }



    }

    Histogram_segmentation(Global_Scharr_hist_percentile);*/
}

void MainWindow::on_Local_Scharr_clicked()
{
    thresh_met = LOCAL_SCHARR;

    //hide the different group boxes and show the global sobel one.
    ui->Global_Sobel_box->hide();
    ui->Local_Sobel_box->hide();
    ui->Global_Scharr_box->hide();
    ui->Local_Scharr_box->show();
    ui->Global_Otsu_box->hide();
    ui->Local_Otsu_box->hide();
    ui->Thresholding_box->hide();
    ui->Adaptive_Thresholding_box->hide();

    /*if((processed_image.channels()==1) & (!processed_image.empty()))
    {
        int colsize = Segmented_image.cols;
        int rowsize = Segmented_image.rows;

        cv::Mat sub_image,sub_image2;

         Segmented_image = processed_image.clone();
         if(!(ui->Local_Scharr_dx_checkBox->isChecked() & ui->Local_Scharr_dy_checkBox->isChecked()))
         {
             Segmented_image.convertTo(Segmented_image,CV_32F);
             sub_image.create(rowsize/Local_Scharr_numberofSubImages,colsize/Local_Scharr_numberofSubImages,CV_32F);
             sub_image2.create(rowsize/Local_Scharr_numberofSubImages,colsize/Local_Scharr_numberofSubImages,CV_32F);
         }
         else
         {
             sub_image.create(rowsize/Local_Scharr_numberofSubImages,colsize/Local_Scharr_numberofSubImages,CV_8U);
             sub_image2.create(rowsize/Local_Scharr_numberofSubImages,colsize/Local_Scharr_numberofSubImages,CV_8U);
         }


         int c,d,e,f;
         for(int a = 0;a<Local_Scharr_numberofSubImages;a++)
         {
             if(a == 0)
             {
                 c=0;
                 d = floor(1.0*rowsize/Local_Scharr_numberofSubImages)-1;
             }else
             {
                 c = d+1;
                 d = d+floor(1.0*rowsize/Local_Scharr_numberofSubImages);
             }

             for(int b = 0;b<Local_Scharr_numberofSubImages;b++)
             {
                 if(b == 0)
                 {
                     e=0;
                     f = floor(1.0*colsize/Local_Scharr_numberofSubImages)-1;
                 }else
                 {
                     e = f+1;
                     f = f+floor(1.0*colsize/Local_Scharr_numberofSubImages);
                 }
                 sub_image = Segmented_image(cv::Rect(e,c,sub_image.cols,sub_image.rows));
                 sub_image.copyTo(sub_image2);
                 //converting to float and doing sobel operation with Scharr mask
                // cv::Sobel(sub_image2,sub_image,CV_32F,1,0,CV_SCHARR,1,0,cv::BORDER_REPLICATE);//need float to do sobel??

                 if(ui->Local_Scharr_dx_checkBox->isChecked() & ui->Local_Scharr_dy_checkBox->isChecked())
                 {
                     //klikker, må gjøre om koncerteringa av Segmenyted image til cv32F, kan ikke komme i linje 514 hvis dx og dy,
                     cv::Mat seg_dx,seg_dy;
                     cv::Mat abs_seg_x,abs_seg_y;
                     cv::Sobel(sub_image2,seg_dx,CV_32F,1,0,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
                     cv::Sobel(sub_image2,seg_dy,CV_32F,0,1,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
                     cv::convertScaleAbs(seg_dx,abs_seg_x);
                     cv::convertScaleAbs(seg_dy,abs_seg_y);
                     cv::addWeighted(abs_seg_x,0.5,abs_seg_y,0.5,0,sub_image);//not perfect
                     qDebug() << "bege";
                 }
                 else if(ui->Local_Scharr_dx_checkBox->isChecked())
                 {
                    cv::Sobel(sub_image2,sub_image,CV_32F,1,0,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
                    //Segmented_image.convertTo(Segmented_image,CV_8U);
                    qDebug() << "dx";
                 }
                 else if(ui->Local_Scharr_dy_checkBox->isChecked())
                 {
                     cv::Sobel(sub_image2,sub_image,CV_32F,0,1,CV_SCHARR,1,0,cv::BORDER_REPLICATE);
                     //Segmented_image.convertTo(Segmented_image,CV_8U);
                     qDebug() << "dy";
                 }



                 }
         }
    }

    if(!(ui->Local_Scharr_dx_checkBox->isChecked() & ui->Local_Scharr_dy_checkBox->isChecked()))
    {
        Segmented_image.convertTo(Segmented_image,CV_8U);
    }

    Histogram_segmentation(Local_Scharr_hist_percentile);*/
}

void MainWindow::on_Global_Otsu_clicked()
{
    thresh_met = GLOBAL_OTSU;
    //hide the different group boxes and show the global sobel one.
    ui->Global_Sobel_box->hide();
    ui->Local_Sobel_box->hide();
    ui->Global_Scharr_box->hide();
    ui->Local_Scharr_box->hide();
    ui->Global_Otsu_box->show();
    ui->Local_Otsu_box->hide();
    ui->Thresholding_box->hide();
    ui->Adaptive_Thresholding_box->hide();

    //Segment the image using Otsus method
    if((processed_image.channels()==1) & (!processed_image.empty()))
    {
        Segmented_image = processed_image.clone();
        cv::threshold(processed_image,Segmented_image,0,255,cv::THRESH_BINARY | cv::THRESH_OTSU);
    }

}
void MainWindow::on_Local_Otsu_clicked()
{
    thresh_met = LOCAL_OTSU;

    //hide the different group boxes and show the global sobel one.
    ui->Global_Sobel_box->hide();
    ui->Local_Sobel_box->hide();
    ui->Global_Scharr_box->hide();
    ui->Local_Scharr_box->hide();
    ui->Global_Otsu_box->hide();
    ui->Local_Otsu_box->show();
    ui->Thresholding_box->hide();
    ui->Adaptive_Thresholding_box->hide();

    if((processed_image.channels()==1) & (!processed_image.empty()))
    {
        //Segmented_image.create(processed_image.size(),processed_image.type());
        Segmented_image = processed_image.clone();
        int colsize = processed_image.cols;
        int rowsize = processed_image.rows;

        cv::Mat sub_image,sub_image2;
        sub_image.create(rowsize/Local_Otsu_numberofSubImages,colsize/Local_Otsu_numberofSubImages,CV_8U);
        sub_image2.create(rowsize/Local_Otsu_numberofSubImages,colsize/Local_Otsu_numberofSubImages,CV_8U);
        int c,d,e,f;
        for(int a = 0;a<Local_Otsu_numberofSubImages;a++)
        {
            if(a == 0)
            {
                c=0;
                d = floor(1.0*rowsize/Local_Otsu_numberofSubImages)-1;
            }else
            {
                c = d+1;
                d = d+floor(1.0*rowsize/Local_Otsu_numberofSubImages);
            }

            for(int b = 0;b<Local_Otsu_numberofSubImages;b++)
            {
                if(b == 0)
                {
                    e=0;
                    f = floor(1.0*colsize/Local_Otsu_numberofSubImages)-1;
                }else
                {
                    e = f+1;
                    f = f+floor(1.0*colsize/Local_Otsu_numberofSubImages);
                }

                sub_image = Segmented_image(cv::Rect(e,c,sub_image.cols,sub_image.rows));
                sub_image.copyTo(sub_image2);
                cv::threshold(sub_image2,sub_image,0,255,cv::THRESH_BINARY | cv::THRESH_OTSU);

                }
        }
    }

}

void MainWindow::on_Thresholding_clicked()
{
    thresh_met = THRESHOLDING;

    //hide the different group boxes and show the thresholding one.
    ui->Global_Sobel_box->hide();
    ui->Local_Sobel_box->hide();
    ui->Global_Scharr_box->hide();
    ui->Local_Scharr_box->hide();
    ui->Global_Otsu_box->hide();
    ui->Local_Otsu_box->hide();
    ui->Thresholding_box->show();
    ui->Adaptive_Thresholding_box->hide();

    if((processed_image.channels()==1) & (!processed_image.empty()))
    {
        processed_image.copyTo(Segmented_image);
        int nl = Segmented_image.rows;
        int nc = Segmented_image.cols;
        if(Segmented_image.isContinuous())
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = Segmented_image.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                if(data[i]>=Threshold)//bright pixels
                {
                    data[i]=255;
                }
                else
                {
                    data[i] = 0;
                }
            }
        }

    }

}

void MainWindow::on_Adaptive_Thresholding_clicked()
{
    thresh_met = ADAPTIVE_THRESHOLDING;

    //hide the different group boxes and show the thresholding one.
    ui->Global_Sobel_box->hide();
    ui->Local_Sobel_box->hide();
    ui->Global_Scharr_box->hide();
    ui->Local_Scharr_box->hide();
    ui->Global_Otsu_box->hide();
    ui->Local_Otsu_box->hide();
    ui->Thresholding_box->hide();
    ui->Adaptive_Thresholding_box->show();



    if((processed_image.channels()==1) & (!processed_image.empty()))
    {
        processed_image.copyTo(Segmented_image);
        int C = ui->Adaptive_Thresholding_C_slider->value();
        if(ui->Adaptive_Thresholding_gaussian_radioButton->isChecked())
        {
            cv::adaptiveThreshold(Segmented_image,Segmented_image,255.0,cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                                  cv::THRESH_BINARY,Adaptive_Thresholding_kernel_size,C);
        }
        else if(ui->Adaptive_thresholding_mean_radioButton->isChecked())
        {
            cv::adaptiveThreshold(Segmented_image,Segmented_image,255.0,cv::ADAPTIVE_THRESH_MEAN_C,
                                  cv::THRESH_BINARY,Adaptive_Thresholding_kernel_size,C);

        }
        //Adaptive_Thresholding_kernel_size

    }

}





void MainWindow::on_Local_Sobel_horizontalSlider_valueChanged(int value)
{
    Local_Sobel_numberofSubImages = value;
    ui->Local_Sobel_lcdNumber->display(value*value);
    //qDebug() << "verdien er: " << Local_Sobel_numberofSubImages;
}

void MainWindow::on_Local_Scharr_horizontalSlider_valueChanged(int value)
{
    Local_Scharr_numberofSubImages = value;
    ui->Local_Scharr_lcdNumber->display(value*value);
}

void MainWindow::on_Local_Otsu_horizontalSlider_valueChanged(int value)
{
    Local_Otsu_numberofSubImages = value;
    ui->Local_Otsu_lcdNumber->display(value*value);
}

void MainWindow::on_Capture_clicked()
{
    if(!Segmented_image.empty())
    {
        imwrite( "Segmented_Image.png", Segmented_image );
    }

    if(!hole_detected_image.empty())
    {
        imwrite( "Hole_detected_Image.png", hole_detected_image );
    }


}

void MainWindow::on_Global_Sobel_histogram_slider_valueChanged(int value)
{
    Global_Sobel_hist_percentile = value;
}

void MainWindow::on_Local_Sobel_histogram_slider_valueChanged(int value)
{
    Local_Sobel_hist_percentile = value;
}

void MainWindow::on_Global_Scharr_histogram_slider_valueChanged(int value)
{
    Global_Scharr_hist_percentile = value;
}

void MainWindow::on_Local_Scharr_histogram_slider_valueChanged(int value)
{
    Local_Scharr_hist_percentile = value;
}

void MainWindow::on_Threshold_slider_valueChanged(int value)
{
    Threshold = value;
}

void MainWindow::on_No_Mode_clicked()
{
    mode = NO_MODE;
}

void MainWindow::on_Hole_Detection_clicked()
{
    mode = HOLE_DETECTION;
}

void MainWindow::on_Growth_detection_clicked()
{
    mode = GROWTH_DETECTION;
}

void MainWindow::custom_XYS_image()
{
    XYZ_image.create(image.size(), image.type());
    int nl = image.rows;
    int nc = image.cols;
    if(XYZ_image.isContinuous())
    {
        nc = nc*nl;
        nl = 1; //1D array;
    }

    //loop exectued only once if the image is continious
    for(int j = 0; j<nl;j++)
    {
        uchar* data = XYZ_image.ptr<uchar>(j);
        uchar* old_data = image.ptr<uchar>(j);
        for(int i = 0; i<nc;i++)
        {

            data[3*i] = 0.607*old_data[3*i] + 0.174*old_data[3*i+1]+0.200*old_data[3*i+2];//X channel
            data[3*i+1] = 0.299*old_data[3*i] + 0.587*old_data[3*i+1]+0.114*old_data[3*i+2];//Y channel
            data[3*i+2] = 0.066*old_data[3*i+1]+1.111*old_data[3*i+2];//Z channel

        }
    }
}

void MainWindow::Histogram_segmentation(int desired_percentage)
{

    cv::Mat hist;
    int histSize = 256;
    float ranges[] = {0,256};
    const float* histRange = {ranges};
    bool uniform = true;
    bool accumulate = false;

    cv::calcHist(&Segmented_image,1,0,cv::Mat(),hist,1,&histSize,&histRange,uniform,accumulate);

    int hist_w = 512; int hist_h = 400;
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


    }

    //QImage hist_img = QImage((const unsigned char*)(histImage.data),histImage.cols,histImage.rows,QImage::Format_Indexed8 );
    //ui->histogram_label->setPixmap(QPixmap::fromImage(hist_img));


    //percentile2i from Digital image processing using matlab

    float sum = 0;
    float* data = hist.ptr<float>(0);
    for (int i = 0;i<256;i++)
    {
        sum = sum+data[i];
        //sum = sum+hist.at<float>(i);
    }

   // float new_sum = 0;
   // data = hist.ptr<float>(0);
    for(int i = 0;i<256;i++)
    {
        data[i] = data[i]/sum;
        //hist.at<float>(i) = hist.at<float>(i)/sum;
      //  new_sum = new_sum+hist.at<float>(i);

    }

     //cumulative sum
    for(int i = 1;i<256;i++ )
    {
        data[i] = data[i]+data[i-1];
       // hist.at<float>(i) = hist.at<float>(i)+hist.at<float>(i-1);
    }

    //finding the index where the cummulative sum is equal to or greather than the changeable variabel
    float changeable_variable = (float)desired_percentage/100;
    float idx = 0;
    for (int i = 0;i<256;i++)
    {
        //if(hist.at<float>(i) >= changeable_variable)
        if(data[i] >= changeable_variable)
        {
           // qDebug() << "fant raden, raden er: " << i << "  verdien er: " << hist.at<float>(i) << "variablene er: " << changeable_variable;
            idx = i;
            break;
        }
    }

    int nl = Segmented_image.rows;
    int nc = Segmented_image.cols;
    if(Segmented_image.isContinuous())
    {
        nc = nc*nl;
        nl = 1; //1D array;
    }

    //loop exectued only once if the image is continious
    for(int j = 0; j<nl;j++)
    {
        uchar* data = Segmented_image.ptr<uchar>(j);

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
}

void MainWindow::Hole_Detection()
{
    //lager en QList hvor størrelsen på regionene blir lagt inn
    //der kan man senere finne avvik fra normal region størrelse for å bestemme hull
    //enten lage ny liste per bilde eller global som husker gamle verdier, hmmmm.....
    QList<int> region_size_list;
    QList<cv::Vec6i> region_list;//[uppermost,furthest down,leftmost,rightmost,area,0]
    cv::Vec6i region;//kan bygge om til 4i, hvis nødvendig fordi topmost er den vi sender inn
    Segmented_image.copyTo(hole_detected_image);
    //qDebug() << "start of one image";
    int area;

    for(int j = 1;j<hole_detected_image.rows-1;j++)
    {
        uchar* data_row = hole_detected_image.ptr<uchar>(j);
        for(int i =1;i<hole_detected_image.cols-1;i++)
        {
            if(data_row[i] == 0)
            {
                region = Region_Growing(j,i);
                area = region[4];
                if(area>500)//to prevent small pixel areas to count towards the median area, make it a changeable variable?
                {
                    region_size_list.append(area);
                    region_list.append(region);
                }
            }
        }
    }


    qSort(region_size_list.begin(),region_size_list.end());

    foreach(cv::Vec6i a ,region_list)
    {
        region_list.removeFirst();//removes the item being examined
        //if the area in question is over three times the median area of the region_size_list then draw a circle around it
        if(a(4) >=  3*region_size_list[(int)region_size_list.size()/2])
        {
            //qDebug() << "fant største region, den er: " << a(4) << "med x,x,y,y" <<a(0) <<a(1) <<a(2) <<a(3) << a(5) ;

            //drawing on image

           // qDebug() << "fant en region som er over tre ganger større enn medianen, den er: " << a(4) << "med x,x,y,y" <<a(0) <<a(1) <<a(2) <<a(3) << a(5) ;
            cv::circle(hole_detected_image,cv::Point(a(2)+(a(3)-a(2))/2,a(0)+(a(1)-a(0))/2),(int)(a(1)-a(0))/2,0,4,8,0);

        }
    }

   // qDebug() << "lista:";
   // qDebug() << region_size_list;

}

cv::Vec6i MainWindow::Region_Growing(int x, int y)
{
    int area = 1;
    QList<cv::Vec2i> growing_list;
    growing_list.append(cv::Vec2i(x,y));
    cv::Vec6i returned_param;//[uppermost,furthest down,leftmost,rightmost,area,0]
    //returned_param = (x,x,y,y,area,0);
    returned_param[0] = x;
    returned_param[1] = x;
    returned_param[2] = y;
    returned_param[3] = y;
    returned_param[4] = area;
    returned_param[5] = 0;
    //growing_list.append(cv::Vec2i(5,6));
    hole_detected_image.at<uchar>(x,y) = 127;
    bool change = true;


    //grow region and keep the pixels that are leftmost rightmost and furthest down, pixelnuber not coordinate
    while(change)
    {
        change = false;
        foreach(cv::Vec2i a ,growing_list)
        {
            growing_list.removeFirst();//removes the pixel being examined

          /*  if((a(0)>0) & (a(1)>0) & (a(0)<(hole_detected_image.rows-1)) & (a(1)<(hole_detected_image.cols-1)) ) //dont use mask on border pixels.
            {
                for(int i=-1 ; i<=1 ; i++)
                    for(int j=-1 ; j<=1 ; j++)
                    {
                        if((hole_detected_image.at<uchar>(a(0)+i,a(1)+j) == 0) & (abs(i)!=abs(j)))//4 connectivity
                        {
                            change = true;
                            growing_list.append(cv::Vec2i(a(0)+i,a(1)+j));
                            hole_detected_image.at<uchar>(a(0)+i,a(1)+j) = 127;//value between 0 and 255
                            area = area+1;

                        }
                    }

            }*/

            if((a(0)>0) & (a(1)>0) & (a(0)<(hole_detected_image.rows-1)) & (a(1)<(hole_detected_image.cols-1)) ) //dont use mask on border pixels.
            {
                uchar* previous_row = hole_detected_image.ptr<uchar>(a(0)-1);
                uchar* current_row = hole_detected_image.ptr<uchar>(a(0));
                uchar* next_row = hole_detected_image.ptr<uchar>(a(0)+1);
                if(previous_row[a(1)]==0)
                {
                    change = true;
                    growing_list.append(cv::Vec2i(a(0)-1,a(1)));
                    area = area+1;
                    previous_row[a(1)] = 127;
                    if(a(0)-1> returned_param[1])//checking topmost, should never come into this one
                    {
                        returned_param[1]=a(0)-1;
                       //qDebug() << "kom inn i høyere rad enn vi begynnte med";
                    }
                }
                if(current_row[a(1)-1]==0)
                {
                    change = true;
                    growing_list.append(cv::Vec2i(a(0),a(1)-1));
                    area = area+1;
                    current_row[a(1)-1] = 127;
                    if( (a(1)-1)<returned_param[2])//checking leftmost
                    {
                        returned_param[2]= a(1)-1;
                    }
                }
                if(current_row[a(1)+1]==0)
                {
                    change = true;
                    growing_list.append(cv::Vec2i(a(0),a(1)+1));
                    area = area+1;
                    current_row[a(1)+1] = 127;
                    if( (a(1)+1)>returned_param[3])//checking rightmost
                    {
                        returned_param[3]= a(1)+1;
                    }
                }
                if(next_row[a(1)]==0)
                {
                    change = true;
                    growing_list.append(cv::Vec2i(a(0)+1,a(1)));
                    area = area+1;
                    next_row[a(1)] = 127;
                    if(a(0)+1> returned_param[1])//checking lowest
                    {
                        returned_param[1]=a(0)+1;
                    }
                }

            }

        }

    }

    returned_param[4]=area;
    return returned_param;
    //return growing_list;

}

void MainWindow::on_y_clicked()
{
    if(!XYZ_image.empty())
    {
        processed_image.create(XYZ_image.size(), CV_8U);
        int nl = XYZ_image.rows;
        int nc = XYZ_image.cols;
        if(processed_image.isContinuous())//unneccessary
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = processed_image.ptr<uchar>(j);
            uchar* old_data = XYZ_image.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                data[i] = 255*((double)old_data[3*i+1])/(double)(old_data[3*i]+old_data[3*i+1]+old_data[3*i+2]) ;

            }
        }
    }
    cspace = Y;



}

void MainWindow::on_x_clicked()
{
    if(!XYZ_image.empty())
    {
        processed_image.create(XYZ_image.size(), CV_8U);
        int nl = XYZ_image.rows;
        int nc = XYZ_image.cols;
        if(processed_image.isContinuous())//unneccessary
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = processed_image.ptr<uchar>(j);
            uchar* old_data = XYZ_image.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                data[i] = 255*((double)old_data[3*i])/(double)(old_data[3*i]+old_data[3*i+1]+old_data[3*i+2]) ;

            }
        }

    }
    cspace = X;

}

void MainWindow::on_Y_clicked()
{
    if(!XYZ_image.empty())
    {
        processed_image.create(XYZ_image.size(), CV_8U);
        int nl = XYZ_image.rows;
        int nc = XYZ_image.cols;
        if(processed_image.isContinuous())//unneccessary
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = processed_image.ptr<uchar>(j);
            uchar* old_data = XYZ_image.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                data[i] = old_data[3*i+1];
            }
        }
    }
    cspace = LUMINANCE;

}

void MainWindow::on_Red_clicked()
{
    if(!image.empty())
    {
        processed_image.create(image.size(), CV_8U);
        int nl = image.rows;
        int nc = image.cols;
        if(processed_image.isContinuous())//unneccessary
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = processed_image.ptr<uchar>(j);
            uchar* old_data = image.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                data[i] = old_data[3*i];

            }
        }

    }

     cspace = RED;

}
void MainWindow::on_Green_clicked()
{
    if(!image.empty())
    {
        processed_image.create(image.size(), CV_8U);
        int nl = image.rows;
        int nc = image.cols;
        if(processed_image.isContinuous())//unneccessary
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = processed_image.ptr<uchar>(j);
            uchar* old_data = image.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                data[i] = old_data[3*i+1];

            }
        }
    }
     cspace = GREEN;

}

void MainWindow::on_Blue_clicked()
{
    if(!image.empty())
    {
        processed_image.create(image.size(), CV_8U);
        int nl = image.rows;
        int nc = image.cols;
        if(processed_image.isContinuous())//unneccessary
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = processed_image.ptr<uchar>(j);
            uchar* old_data = image.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                data[i] = old_data[3*i+2];

            }
        }
    }
     cspace = BLUE;

}

void MainWindow::on_x2_clicked()
{
    if(!XYZ2.empty())
    {
        processed_image.create(XYZ2.size(), CV_8U);
        int nl = XYZ2.rows;
        int nc = XYZ2.cols;
        if(processed_image.isContinuous())//unneccessary
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = processed_image.ptr<uchar>(j);
            uchar* old_data = XYZ2.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                data[i] = 255*((double)old_data[3*i])/(double)(old_data[3*i]+old_data[3*i+1]+old_data[3*i+2]) ;
            }
        }
    }
    cspace = X2;

}

void MainWindow::on_y2_clicked()
{
    if(!XYZ2.empty())
    {
        processed_image.create(XYZ2.size(), CV_8U);
        int nl = XYZ2.rows;
        int nc = XYZ2.cols;
        if(processed_image.isContinuous())//unneccessary
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = processed_image.ptr<uchar>(j);
            uchar* old_data = XYZ2.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                data[i] = 255*((double)old_data[3*i+1])/(double)(old_data[3*i]+old_data[3*i+1]+old_data[3*i+2]) ;
            }
        }
    }
    cspace = Y2;

}
void MainWindow::on_Y2_clicked()
{
    if(!XYZ2.empty())
    {
        processed_image.create(XYZ2.size(), CV_8U);
        int nl = XYZ2.rows;
        int nc = XYZ2.cols;
        if(processed_image.isContinuous())//unneccessary
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = processed_image.ptr<uchar>(j);
            uchar* old_data = XYZ2.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                data[i] = old_data[i*3+1];
            }
        }
    }
    cspace = LUMINANCE2;

}

void MainWindow::on_Lab_clicked()
{
    cspace = LAB;
}

void MainWindow::on_L_clicked()
{
    if(!Lab_image.empty())
    {
        processed_image.create(Lab_image.size(), CV_8U);
        int nl = Lab_image.rows;
        int nc = Lab_image.cols;
        if(processed_image.isContinuous())//unneccessary
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = processed_image.ptr<uchar>(j);
            uchar* old_data = Lab_image.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                data[i] = 2.55*old_data[3*i];
            }
        }
    }
    cspace = L;

}

void MainWindow::on_a_clicked()
{
    if(!Lab_image.empty())
    {
        processed_image.create(Lab_image.size(), CV_8U);
        int nl = Lab_image.rows;
        int nc = Lab_image.cols;
        if(processed_image.isContinuous())//unneccessary
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = processed_image.ptr<uchar>(j);
            uchar* old_data = Lab_image.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                data[i] = 1.0625*(120+old_data[3*i+1]);
            }
        }
    }
    cspace = A;

}

void MainWindow::on_b_clicked()
{
    if(!Lab_image.empty())
    {
        processed_image.create(Lab_image.size(), CV_8U);
        int nl = Lab_image.rows;
        int nc = Lab_image.cols;
        if(processed_image.isContinuous())//unneccessary
        {
            nc = nc*nl;
            nl = 1; //1D array;
        }

        //loop exectued only once if the image is continious
        for(int j = 0; j<nl;j++)
        {
            uchar* data = processed_image.ptr<uchar>(j);
            uchar* old_data = Lab_image.ptr<uchar>(j);
            for(int i = 0; i<nc;i++)
            {
                data[i] = 1.0625*(120+old_data[3*i+2]);
            }
        }
    }
    cspace = B;

}

void MainWindow::on_Dilation_horizontalSlider_valueChanged(int value)
{
    Dilation_iterations = value;
}



void MainWindow::on_Gaussian_horizontalSlider_valueChanged(int value)
{
    if(value % 2)
    {
        Gaussian_kernel_size = value;
        ui->Gaussian_lcdNumber->display(value);
    }

}

void MainWindow::on_Global_Sobel_dx_slider_valueChanged(int value)
{
    Global_Sobel_dx = value;
}

void MainWindow::on_Global_Sobel_dy_slider_valueChanged(int value)
{
    Global_Sobel_dy = value;
}

void MainWindow::on_Global_Sobel_kernel_slider_valueChanged(int value)
{
    if(value % 2)
    {
        Global_Sobel_kernel_size = value;
        ui->Global_Sobel_kernel_lcdNumber->display(value);
       // qDebug() << value;
    }
}


void MainWindow::on_Local_Sobel_dx_slider_valueChanged(int value)
{
    Local_Sobel_dx = value;

}

void MainWindow::on_Local_Sobel_dy_slider_valueChanged(int value)
{
    Local_Sobel_dy = value;

}

void MainWindow::on_Local_Sobel_kernel_slider_valueChanged(int value)
{
    if(value % 2)
    {
        Local_Sobel_kernel_size = value;
        ui->Local_Sobel_kernel_lcdNumber->display(value);
    }
}


void MainWindow::on_Adaptive_Thresholding_kernel_slider_valueChanged(int value)
{
    if(value % 2)
    {
        Adaptive_Thresholding_kernel_size = value;
        ui->Adaptive_Thresholding_kernel_lcdNumber->display(value);
    }

}
