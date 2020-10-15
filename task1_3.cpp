#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
 
using namespace cv;
using namespace std;
Point2f test[5];

 void CodeRota(double x,double y,double theraz,double &outx,double &outy)
 {
	 double x1=x;
	 double y1=y;
	 double rz=theraz*3.14/180;
	 outx=cos(rz)*x1-sin(rz)*y1;
	 outy=sin(rz)*x1+cos(rz)*y1;
 }
 double Cross(Point2f a,Point2f b)
 {
	 return a.x*b.y-a.y*b.x;
 }
 double getmj(Point2f a[])
 {
	double sum=0;
	a[4]=a[0];
	for(int i=0;i<4;i++)
	{
		sum+=Cross(a[i],a[i+1]);
	}
	sum/=2.0;
	return sum;
 }
int main() 
{
	double camD[9]={3051.606376040263, 0, 2007.232966889607,0, 3051.517074095969, 1512.576722452797,0, 0, 1};
	double distCoeffD[5] ={-0.0290066937837023, 0.355639115843682, 0.0001950203154208102, -0.002119098642681893, -0.6993275354059444};
	Mat cameraMatrix=Mat(3,3,CV_64FC1,camD);
	Mat distCoeffs = Mat(5,1,CV_64FC1,distCoeffD); 
	//cout<<cameraMatrix<<endl;
	//cout<<distCoeffs<<endl;
   string   filename;
    cin>>filename;
    Mat src=imread(filename);
	//imshow("22",src);
	Mat src1,binary;
	//Mat igs=src.clone();
	GaussianBlur(src, src1, Size(5, 5), 1.5);
	//namedWindow("gauss",0);
	//imshow("gauss",src1);
	//waitKey(0);
    cvtColor(src1,src1,COLOR_BGR2GRAY);
	//threshold(src1, src1, 180, 255, THRESH_BINARY);
	 //dilate(src1, src1, Mat());
	
	int  thresh=100;
	Canny(src1, binary, thresh, thresh * 3, 3);
	// namedWindow("222",0);
	//imshow("222",binary);
	
	//binary=~binary;
	
	//inRange(src1,Scalar(0,0,0),Scalar(180,255,46),binary);
	vector<vector<Point>> contours;
     vector<Vec4i> hireachy;
	 findContours(binary, contours, hireachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());
	 vector<Point>point;
	 int xb=0;
	 //cout<<contours.size()<<endl;
	 for(int t=0;t<contours.size();t++)
	 {
		   int epsilon = 0.01 * arcLength(contours[t], true);
            approxPolyDP(contours[t], point, epsilon, true);
			if(point.size()==4)
			{
				for(int ii=0;ii<4;ii++)
				test[ii]=point[ii];
				double mj=getmj(test);
				if(mj>1000)
				{
						xb=t;
					break;
				}
			
			}
	 }
          drawContours(src, contours, xb, Scalar(0, 0, 255), 5, 8, Mat(), 0, Point());
		  //waitKey(0);

int min1 = 0;
int min2 = 0;
int max1= 0;
int max2 = 0;
double max=0;
double min=point[0].x+point[0].y;
for (int m = 1; m < point.size(); m++)
{
		double add=point[m].x+point[m].y;
		if(add<min)
		{
			min1=m;
			min=add;
		}
		if(add>max)
		{
			max1=m;
			max=add;
		}
}
for (int m = 0; m < point.size(); m++)
{
    if ((m != min1) && (m != max1))
    {
        if (point[m].x>point[m].y)
        {
            
            min2=m;
        }
        if (point[m].y > point[m].x)
        {
            max2= m;
        }
    }
}
vector<Point2f>points;
points.clear();
points.push_back(point[min1]);
points.push_back(point[min2]);
points.push_back(point[max1]);
points.push_back(point[max2]);

//cout<<points<<endl;

		    vector<Point3f> objP;
			Mat objM;
   			objP.clear();
    		objP.push_back(Point3f(0, 0, 0));
   		 	objP.push_back(Point3f(10, 0, 0));
    		objP.push_back(Point3f(10, 10, 0));
    		objP.push_back(Point3f(0, 10, 0));
			Mat(objP).convertTo(objM,CV_32F);

			Mat  rvec=Mat::zeros(3,1,CV_64FC1);  
			Mat  tvec=Mat::zeros(3,1,CV_64FC1);
			solvePnP(objM, Mat(points), cameraMatrix, distCoeffs, rvec, tvec);
			//cout<<rvec<<endl;
			double rm[3][3];
			Mat rotMat(3,3,CV_64FC1,rm);
			Rodrigues(rvec,rotMat);
			//cout<<tvec<<endl;
			//cout<<rotMat<<endl;
			//float theta_z=atan2(rm[1][0],rm[0][0])*57.2958;
		//float theta_y=atan2(-rm[2][0],sqrt(rm[2][0]*rm[2][0]+rm[2][2]*rm[2][2]))*57.2958;
			//float theta_x=atan2(rm[2][1],rm[2][2])*57.2958;
			//double tx=tvec.ptr<double>(0)[0];
			//double ty=tvec.ptr<double>(0)[1];
			double tz=tvec.ptr<double>(0)[2];
			//CodeRota(tx,ty,-1*theta_z,tx,ty);
			printf("%.2f cm\n",tz);
			int ans=(int)(tz*100+0.5);
			int ansf=ans/100;
			int anst=ans%10;
			int anstt=(ans/10)%10;
			string s=to_string(ansf)+"."+to_string(anstt)+to_string(anst);
			putText(src,s,points[0],FONT_HERSHEY_COMPLEX, 2, Scalar(0, 0, 0), 2, 8, 0);
			 namedWindow("1",0);
		  	imshow("1",src);
			waitKey(0);
	return  0;
}