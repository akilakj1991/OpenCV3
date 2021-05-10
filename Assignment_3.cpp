
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include <string.h>
#include <cv.h>
#include <highgui.h>

using namespace cv;
using namespace std;


Mat img;
Mat src; Mat src_gray;


RNG rng(12345);

float rotation;
float translation;
float Tx, Ty, Tz;

void Transmormation_function(string);  // The transformation function 

void  main()  // Main function only pass the corresponding image in the trasformation function as the argument 
{
	while (true) {
		namedWindow("waitkey window", CV_WINDOW_NORMAL);

		cout << "\npress 1,2,3,4,5 to view image or ESC again to quit program\n" << endl;
		switch (waitKey(0)) {
		case 49: 
			Transmormation_function("Image 1.jpg");  //putting image as the function's argument 
			break;
		case 50:
			Transmormation_function("Image 2.jpg");
			break;
		case 51:
			Transmormation_function("Image 3.jpg");
			break;
		case 52:
			Transmormation_function("Image 4.jpg");
			break;
		case 53:
			Transmormation_function("Image 5.jpg");
			break;
		case 27: 
			cout << "Thank You";    // When esc press only ones 
			waitKey(2000);   // Makes a delay of approximately 2 seconds 
			return;
		}
	}
	return;
}

// transform function
void Transmormation_function(string argv)
{
	Mat threshold_output;     // Variable to store threshold image
	vector<vector<Point> > contours;  //vector veriable for storing contours
	vector<Vec4i> hierarchy;

	/// Load source image and convert it to gray
	src = imread(argv, 1);   
	resize(src, src, Size(), 0.7, .7, CV_INTER_AREA);

	/// Convert image to gray and blur it
	cvtColor(src, src_gray, CV_BGR2GRAY);
	//  smooth image
	blur(src_gray, src_gray, Size(3, 3));

	/// Detect edges using canny
	Canny(src_gray, threshold_output, 150, 300, 3);
	// remove noise frrom backgorund
	dilate(threshold_output, threshold_output, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(threshold_output, threshold_output, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));


	/// Find contours
	findContours(threshold_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));
	// eliminates image 3's white letter A
	if (contours.size() > 2) {
		contours.erase(contours.begin() + 1, contours.end() - 1);
	}

	// Find the rotated rectangles contour
	vector<RotatedRect> minRect(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		minRect[i] = minAreaRect(Mat(contours[i]));
	}

	// Approximate contours to polygons + get bounding rects and circles
	vector<vector<Point> > contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());
	vector<Point2f>center(contours.size());
	vector<float>radius(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
		boundRect[i] = boundingRect(Mat(contours_poly[i]));
	}

	/// Drawing Image
	Mat drawing = Mat::zeros(threshold_output.size(), CV_8UC3);
	//drawing for each contour
	for (int i = 0; i< 2; i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point());
		rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0);

		// rotated rectangle
		Point2f rect_points[4]; minRect[i].points(rect_points);
		for (int j = 0; j < 4; j++) {
			line(drawing, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
		}
		//drawing centre circle and translation arrow
		circle(drawing, minRect[i].center, 4, color, -1, 8, 0);
		arrowedLine(drawing, minRect[0].center, minRect[1].center, color, 1, 8);
		cout << "center coordinates for contour " << i + 1 << " = " << minRect[i].center << endl;
	}

	// get translation
	double Ax = minRect[1].center.x;
	double Ay = minRect[1].center.y;
	double Bx = minRect[0].center.x;
	double By = minRect[0].center.y;

	Tx = -(Bx - Ax);
	Ty = -(By - Ay);
	Tz = 0;

	translation = sqrt(((Bx - Ax)*(Bx - Ax)) + ((By - Ay)*(By - Ay)));

	//get rotation
	float angle_deg0 = minRect[0].angle;
	if (minRect[0].size.width < minRect[0].size.height) {
		angle_deg0 = 90 + angle_deg0;
	}



	float angle_deg1 = minRect[1].angle;
	if (minRect[1].size.width < minRect[1].size.height) {
		angle_deg1 = 90 + angle_deg1;

	}

	float rotation = (angle_deg0 - angle_deg1);

	double transform[4][4] = {
		{ (cos(rotation)), (sin(rotation)), 0, Tx },
		{ (-sin(rotation)), (cos(rotation)), 0, Ty },
		{ 0, 0, 1, Tz },
		{ 0,0,0,1 }
	};

	cout << "\ntransfomation matrix";
	for (int i = 0; i < 4; i++) {
		printf("\n[");
		for (int j = 0; j < 4; j++) {
			printf("%f, ", transform[i][j]);
		}
		cout << "]";
	}

	cout << "\n\nthe rotation between frames " << rotation << "deg" << endl;
	cout << "the translation between frames " << translation << endl;

	namedWindow("Original", CV_WINDOW_AUTOSIZE);   
	imshow("Original", src); // showing the original image
	imshow(argv, drawing);  // showing the trasformation image
	// imwrite("F:\Image5.JPG", drawing);


	if (waitKey(0) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
	{

		destroyAllWindows();
		//i++;
	}
}
