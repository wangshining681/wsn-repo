#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdio>
#include<vector>
#include<math.h>
#include<string>
#include<algorithm>
//#define MATCHMETHOD TM_SQDIFF_NORMED
using namespace cv;
using namespace std;
using namespace cv::ml;

enum
{
	WIDTH_GREATER_THAN_HEIGHT,
	ANGLE_TO_UP
};

class LightDescriptor
{
public:
	LightDescriptor() {};
	LightDescriptor(const cv::RotatedRect& light)
	{
		width = light.size.width;
		length = light.size.height;
		center = light.center;
		angle = light.angle;
		area = light.size.area();
	}
	const LightDescriptor& operator =(const LightDescriptor& ld)
	{
		this->width = ld.width;
		this->length = ld.length;
		this->center = ld.center;
		this->angle = ld.angle;
		this->area = ld.area;
		return *this;
	}

	/*
	*	@Brief: return the light as a cv::RotatedRect object
	*/
	cv::RotatedRect rec() const
	{
		return cv::RotatedRect(center, cv::Size2f(width, length), angle);
	}

public:
	float width;
	float length;
	cv::Point2f center;
	float angle;
	float area;
};


RotatedRect&  adjustRec(cv::RotatedRect& rec, const int mode)
{
	using std::swap;

	float& width = rec.size.width;
	float& height = rec.size.height;
	float& angle = rec.angle;

	if(mode == WIDTH_GREATER_THAN_HEIGHT)
	{
		if(width < height)
		{
			swap(width, height);
			angle += 90.0;
		}
	}

	while(angle >= 90.0) angle -= 180.0;
	while(angle < -90.0) angle += 180.0;

	if(mode == ANGLE_TO_UP)
	{
		if(angle >= 45.0)
		{
			swap(width, height);
			angle -= 90.0;
		}
		else if(angle < -45.0)
		{
			swap(width, height);
			angle += 90.0;
		}
	}

	return rec;
}



int main()
{
    VideoCapture capture("IMG_2223.mp4");
    //检测视频是否读取成功
    if (!capture.isOpened())
    {
        cout << "No input image";
        return 1;
    }

    //获取图像帧率
    double rate = capture.get(CV_CAP_PROP_FPS);
    bool stop = false;
    Mat frame;
    namedWindow("Example",0);

    int delay = 1000/rate;

    while (!stop)
    {
        if (!capture.read(frame))
            break;
        Mat binimg;
        cvtColor(frame,binimg,CV_BGR2GRAY,1);
        threshold(binimg,binimg,50,255,THRESH_BINARY);
        vector<Mat> channels;
        split(frame,channels);
        
        
        Mat grayimg;
        subtract(channels[2],channels[0],grayimg);
        
        //grayimg=channels.at(2)-channels.at(0);
        
        threshold(grayimg,grayimg,80, 255, cv::THRESH_BINARY);
      
        Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        dilate(grayimg, grayimg, element); 
        Mat img=binimg&grayimg;

        dilate(img,img, element); 
       // namedWindow("1",0);
        //imshow("1",img);
        vector<vector<Point>> lightContours;
       
        findContours(img.clone(),lightContours,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
       // cout<<lightContours.size()<<endl;
        //imshow("1",img);


       vector<LightDescriptor> lightInfos;
        //cout<<lightContours[0]<<endl;
       for(const auto& contour : lightContours)
        {
                //得到面积
            float lightContourArea = contourArea(contour);
            //面积太小的不要
                if(contour.size() <= 5 ||
            lightContourArea < 10) continue;
            //椭圆拟合区域得到外接矩形
                RotatedRect lightRec = fitEllipse(contour);
                //矫正灯条
                adjustRec(lightRec, ANGLE_TO_UP);
                //宽高比、凸度筛选灯条
                if(lightRec.size.width / lightRec.size.height >1.0 ||lightContourArea / lightRec.size.area() < 0.5)continue;
            //对灯条范围适当扩大
            lightRec.size.width *= 1.1;
            lightRec.size.height *= 1.1;
                Rect lightRect = lightRec.boundingRect();
                const Rect srcBound(Point(0, 0), frame.size());
                lightRect &= srcBound;
                Mat lightImg=frame(lightRect);
                Mat lightMask = Mat::zeros(lightRect.size(), CV_8UC1);
                Point2f lightVertexArray[4];
			    lightRec.points(lightVertexArray);
                vector<Point> lightVertex;
                for(int i = 0; i < 4; i++)
                {
                    lightVertex.emplace_back(Point(lightVertexArray[i].x - lightRect.tl().x,
                                                lightVertexArray[i].y - lightRect.tl().y));
                }
			    fillConvexPoly(lightMask, lightVertex, 255);
                if(lightImg.size().area() <= 0 || lightMask.size().area() <= 0) continue;
          
                lightInfos.push_back(LightDescriptor(lightRec));
        }
        //没找到灯条就返回没找到
        
        sort(lightInfos.begin(), lightInfos.end(), [](const LightDescriptor& ld1, const LightDescriptor& ld2)
		{
			return ld1.center.x < ld2.center.x;
		});
    //cout<<lightInfos.size()<<endl;
     LightDescriptor left;
      LightDescriptor right;
      float meanLen =0;
    for(int i = 0; i < lightInfos.size(); i++)
    {//遍历所有灯条进行匹配
	for(int j = i + 1; (j < lightInfos.size()); j++)
          {
            const LightDescriptor& leftLight  = lightInfos[i];
            const LightDescriptor& rightLight = lightInfos[j];
		/*
		*	Works for 2-3 meters situation
		*	morphologically similar: // parallel 
						 // similar height
		*/
              //角差
              float angleDiff_ = abs(leftLight.angle - rightLight.angle);
              //长度差比率
              float LenDiff_ratio = abs(leftLight.length - rightLight.length) / max(leftLight.length, rightLight.length);
              //筛选
              if(angleDiff_ > 7.0 ||LenDiff_ratio > 0.2)
                {
                    continue;
                }

		/*
		*	proper location:  y value of light bar close enough 
		*			  ratio of length and width is proper
		*/
              //左右灯条相距距离
		    float dis =sqrt(pow(leftLight.center.x-rightLight.center.x,2)+pow(leftLight.center.y-rightLight.center.y,2));
              //左右灯条长度的平均值
             meanLen = (leftLight.length + rightLight.length) / 2;
              //左右灯条中心点y的差值
              float yDiff = abs(leftLight.center.y - rightLight.center.y);
              //y差比率
              float yDiff_ratio = yDiff / meanLen;
              //左右灯条中心点x的差值
              float xDiff = abs(leftLight.center.x - rightLight.center.x);
              //x差比率
              float xDiff_ratio = xDiff / meanLen;
              //相距距离与灯条长度比值
              float ratio = dis / meanLen;
              //筛选
              if(yDiff_ratio > 2.0 ||xDiff_ratio < 0.5 ||ratio > 5.0 ||ratio < 1.0)
                {
                    continue;
                }

		// calculate pairs' info 
              //按比值来确定大小装甲
		//int armorType = ratio > 3.2? BIG_ARMOR : SMALL_ARMOR;
		// calculate the rotation score
        float ratiOff=0;
		if(3.2-ratio>ratiOff)
        ratiOff=3.2-ratiOff;
		float yOff = yDiff / meanLen;
		float rotationScore = -(ratiOff * ratiOff + yOff * yOff);
        //cout<<rotationScore<<endl;
              //得到匹配的装甲板
             // ArmorDescriptor armor(leftLight, rightLight, armorType, channels.at(1), rotationScore, _param);
		//_armors.emplace_back(armor);
        left=leftLight;
        right=rightLight;
		break;
	}
}
       //cout<<left.center<<endl;
       //cout<<right.center<<endl;
      Point2f lightline[4];
      lightline[0].x=left.center.x;
       lightline[0].y=left.center.y-meanLen/2;
         lightline[1].x=left.center.x;
       lightline[1].y=left.center.y+meanLen/2;
         lightline[2].x=right.center.x;
       lightline[2].y=right.center.y+meanLen/2;
         lightline[3].x=right.center.x;
       lightline[3].y=right.center.y-meanLen/2;
        for(int i=0;i<4;i++)
        {
             line(frame, lightline[i], lightline[(i+1)%4], Scalar(0,255,0));
        }
        imshow("Example",frame);
        
        if (waitKey(delay) >= 0)
            stop = true;
    }
        return 0;
}