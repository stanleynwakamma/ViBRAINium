//
//  main.cpp
//  DeadShot
//
//  Created by Nile Walker on 9/6/20.
//  Copyright © 2020 Nile Walker. All rights reserved.
//
// #include <iostream>
// #include <algorithm>
// #include <vector>
#include <zbar.h>

#include <opencv2/opencv.hpp>
#include <deque>

using namespace cv;

int getMaxAreaContourId(std::vector<std::vector<cv::Point>> contours)
{
    double maxArea = 0;
    int maxAreaContourId = -1;
    for (int j = 0; j < contours.size(); j++)
    {
        double newArea = cv::contourArea(contours.at(j));
        if (newArea > maxArea)
        {
            maxArea = newArea;
            maxAreaContourId = j;
        } // End if
    }     // End for
    return maxAreaContourId;
} // End function

int main(int, char **)
{
    VideoCapture cap(0); // open the default camera
    if (!cap.isOpened()) // check if we succeeded
        return -1;
    Mat edges;
    namedWindow("Tracking", 1);

    int UpdateEveryNFrames = 6;
    int FramesPassed = 0;
    float BallDiameter = .07; // m

    int FPS = 30.0;
    float TimeStepSize = 1.0 / FPS;

    int EulerSteps = 18;

    Scalar color = Scalar(0, 25, 255);

    Mat frame, hsv_frame, thresh_frame;
    std::deque<Point> Trajectory;
    std::deque<Point> Last_Trajectory;
    std::deque<std::deque<Point>> Predictions;
    std::deque<std::deque<Point>> Last_Predictions;

    for (;;)
    {
        FramesPassed++;
        // Get a new frame from camera
        cap >> frame;
        cvtColor(frame, hsv_frame, COLOR_BGR2HSV);
        inRange(hsv_frame, Scalar(16, 135, 129), Scalar(27, 230, 255), thresh_frame);
        medianBlur(thresh_frame, thresh_frame, 3);

        std::vector<std::vector<Point>> contours;
        findContours(thresh_frame, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

        std::vector<std::vector<Point>> contours_poly(contours.size());
        std::vector<Point2f> centers(contours.size());
        std::vector<float> radius(contours.size());

        for (size_t i = 0; i < contours.size(); i++)
        {
            approxPolyDP(contours[i], contours_poly[i], 3, true);
            minEnclosingCircle(contours_poly[i], centers[i], radius[i]);
        }
        Mat drawing = frame;
        if (contours.size() > 2)
        {
            // Find the Ball by Largest Contour
            int max_contour_id = getMaxAreaContourId(contours);
            circle(drawing, centers[max_contour_id], (int)radius[max_contour_id], color, 2);

            // Update Real Tracking Path
            if (Trajectory.size() < 10)
            {
                Trajectory.push_back(centers[max_contour_id]);
            }
            else
            {
                Trajectory.push_back(centers[max_contour_id]);
                Trajectory.pop_front();
            }
            Point V0 = Point(0, 0);
            //DrawTrueTrajectory(Trajectory,drawing);
            for (unsigned i = 0; i < Trajectory.size() - 1; i++)
            {
                V0 += Point(Trajectory.at(i).x - Trajectory.at(i + 1).x, Trajectory.at(i).y - Trajectory.at(i + 1).y);
                line(drawing, Trajectory.at(i) + Point(10, 0), Trajectory.at(i) + Point(-10, 0), Scalar(255, 0, 0), 6, 8);
                line(drawing, Trajectory.at(i) + Point(0, 10), Trajectory.at(i) + Point(0, -10), Scalar(255, 0, 0), 6, 8);
                line(drawing, Trajectory.at(i), Trajectory.at(i + 1), Scalar(255, 0, 0), 4, 8);
            }
            if (Trajectory.size() > 2)
            {
                V0 = Point(Trajectory.at(Trajectory.size() - 1).x - Trajectory.at(Trajectory.size() - 2).x, Trajectory.at(Trajectory.size() - 1).y - Trajectory.at(Trajectory.size() - 2).y);
            }

            int PixelsPerMeter = (2 * radius[max_contour_id]) / BallDiameter;

            Point start = Trajectory.back();
            Point curr = start;
            Point last = curr;
            //DrawPredictedTrajectory(Trajectory,drawing);
            std::deque<Point> Temp;
            for (int t = 1; t < EulerSteps; t++)
            {
                curr = Point(start.x + V0.x * t, start.y + V0.y * t - .5 * -9.81 * PixelsPerMeter * ((TimeStepSize * t) * (TimeStepSize * t)));
                line(drawing, curr + Point(10, 0), curr + Point(-10, 0), Scalar(255, 0, 255), 6, 8);
                line(drawing, curr + Point(0, 10), curr + Point(0, -10), Scalar(255, 0, 255), 6, 8);
                line(drawing, last, curr, Scalar(255, 0, 255), 4, 8);
                Temp.push_back(curr);
                last = curr;
            }
            if (Predictions.size() < 10)
            {
                Predictions.push_back(Temp);
            }
            else
            {
                Predictions.push_back(Temp);
                Predictions.pop_front();
            }

            if (Trajectory.size() < 10)
            {
                Trajectory.push_back(centers[max_contour_id]);
            }
            else
            {
                Trajectory.push_back(centers[max_contour_id]);
                Trajectory.pop_front();
            }
            //Predictions.push();
            Last_Trajectory = Trajectory;
            Last_Predictions = Predictions;
        }
        else
        {
            if (Last_Trajectory.size() > 0)
            {
                for (unsigned i = 0; i < Last_Trajectory.size() - 1; i++)
                {
                    line(drawing, Last_Trajectory.at(i) + Point(10, 0), Last_Trajectory.at(i) + Point(-10, 0), Scalar(255, 0, 0), 6, 8);
                    line(drawing, Last_Trajectory.at(i) + Point(0, 10), Last_Trajectory.at(i) + Point(0, -10), Scalar(255, 0, 0), 6, 8);
                    line(drawing, Last_Trajectory.at(i), Last_Trajectory.at(i + 1), Scalar(255, 0, 0), 4, 8);
                }
                if (Trajectory.size() > 0)
                {
                    Trajectory.clear();
                }
            }
            if (Last_Predictions.size() > 0)
            {
                for (unsigned i = 0; i < Last_Predictions.size(); i++)
                {
                    for (unsigned j = 0; j < Last_Predictions.at(i).size() - 1; j++)
                    {
                        line(drawing, Last_Predictions.at(i).at(j) + Point(10, 0), Last_Predictions.at(i).at(j) + Point(-10, 0), Scalar(255, 0, 255), 6, 8);
                        line(drawing, Last_Predictions.at(i).at(j) + Point(0, 10), Last_Predictions.at(i).at(j) + Point(0, -10), Scalar(255, 0, 255), 6, 8);
                        line(drawing, Last_Predictions.at(i).at(j), Last_Predictions.at(i).at(j + 1), Scalar(255, 0, 255), 4, 8);
                    }
                }
                if (Predictions.size() > 0)
                {
                    Predictions.clear();
                }
            }
        }
        imshow("Tracking", drawing);
        if (waitKey(30) >= 0)
            break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
// g++ $(pkg-config --cflags --libs opencv4) -std=c++11  sample.cpp -o sample

//Reference:https://www.learnopencv.com/opencv-qr-code-scanner-c-and-python/



#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;
using namespace zbar;

typedef struct
{
  string type;
  string data;
  vector <Point> location;
}decodedObject;

// Find and decode barcodes and QR codes
void decode(Mat &im, vector<decodedObject>&decodedObjects, ImageScanner &scanner)
{

  // Create zbar scanner
  ImageScanner scanner;

  // disable all
  scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 0);

  // enable qr
  scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);

  // Convert image to grayscale
  Mat imGray;
  cvtColor(im, imGray,CV_BGR2GRAY);

  // Wrap image data in a zbar image
  Image image(im.cols, im.rows, "Y800", (uchar *)imGray.data, im.cols * im.rows);

  // Scan the image for barcodes and QRCodes
  int n = scanner.scan(image);

  // Print results
  for(Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
  {
    decodedObject obj;

    obj.type = symbol->get_type_name();
    obj.data = symbol->get_data();

    // Print type and data
    cout << "Type : " << obj.type << endl;
    cout << "Data : " << obj.data << endl << endl;
    decodedObjects.push_back(obj);
  }
}

int main(int argc, char *argv[])
{

  // Read image
  string imagepath = argv[1];
  Mat im = imread(imagepath);

   // Variable for decoded objects
   vector<decodedObject> decodedObjects;

   // Find and decode barcodes and QR codes
   decode(im, decodedObjects);

   return 0;
 }
