#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //setup window
    ui->setupUi(this);
    this->setWindowTitle(QString("System 1, v.0.2.6"));

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
    Naive_threshold = 90;
    Adaptive_Thresholding_kernel_size = 3;
    Adaptive_thresholding_C = 0;
    Dilation_iterations = 1;
    Gaussian_kernel_size = 1;


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
    if(ui->Gaussian_checkBox->isChecked() && !processed_image.empty())
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
                                       ui->Local_Scharr_dx_checkBox->isChecked(),ui->Local_Scharr_dy_checkBox->isChecked(),
                                       ui->Otsu_in_edge_checkBox->isChecked());
        break;
    case GLOBAL_OTSU:
        Segmented_image = Global_Otsu(processed_image);
        break;
    case LOCAL_OTSU:
        Segmented_image = Local_Otsu(processed_image,Local_Otsu_numberofSubImages);
        break;
    case THRESHOLDING:
        Segmented_image = Naive_Thresholding(processed_image,Naive_threshold);
        break;
    case ADAPTIVE_THRESHOLDING:
        Segmented_image = Adaptive_Thresholding(processed_image,Adaptive_Thresholding_kernel_size,Adaptive_thresholding_C,
                                                ui->Adaptive_Thresholding_gaussian_radioButton->isChecked());
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
        hole_detected_image = Hole_detection(Segmented_image);
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
/*    if((processed_image.channels()==1) & (!processed_image.empty()))
    {
        Segmented_image = processed_image.clone();
        cv::threshold(processed_image,Segmented_image,0,255,cv::THRESH_BINARY | cv::THRESH_OTSU);
    }
*/
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
/*
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
*/
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
    Naive_threshold = value;
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

void MainWindow::on_Adaptive_Thresholding_C_slider_valueChanged(int value)
{
    Adaptive_thresholding_C = value;
}
