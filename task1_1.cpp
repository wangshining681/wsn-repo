#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdio>
#define MATCHMETHOD TM_SQDIFF_NORMED
using namespace cv;
using namespace std;


int main(int argc, char** argv)
{
    string fname;
    cin >> fname;
    Mat src = imread(fname);
    Mat src_gray, binary;
    Mat igs = src.clone();
    // Mat Triangle = src.clone(), Rect = src.clone(), BigCircle = src.clone(), SmallCircle = src.clone();
    int tri = 0, rec = 0, cir = 0;
    if (src.empty()) {
        printf("Could not load image...");
        return -1;
    }
    namedWindow("Input Image", 0);
    imshow("Input Image", src);
    //閿熸枻鎷峰�奸敓鏂ゆ嫹
    cvtColor(src, src_gray, COLOR_BGR2HSV);
    int hmin[5] = { 0,0,35,26,100 };
    int hmax[5] = { 180,10,77,34,124 };
    int smin[5] = { 0,43,43,43,43 };
    int smax[5] = { 255,255,255,255,255 };
    int vmin[5] = { 0,46,46,46,46 };
    int vmax[5] = { 46,255,255,255,255 };
    int Rmax[5] = { 0,255,255,0,0 };
    int Gmax[5] = { 0,0,255,255,0 };
    int Bmax[5] = { 0,0,0,0,255 };
    int num = 0;
    for (int i = 0; i < 5; i++)
    {
        int rgbmin = 1;
        if (i == 0)
            rgbmin = 0;
        inRange(src_gray, Scalar(hmin[i], smin[i], vmin[i]), Scalar(hmax[i], smax[i], vmax[i]), binary);
        // threshold(src_gray, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);
         //binary = ~binary;
        //namedWindow("binary", 0);
        //imshow("binary", binary);
        //閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
        vector<vector<Point>> contours;
        vector<Vec4i> hireachy;
        findContours(binary, contours, hireachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());
        //cout << contours.size() << endl;
        //閿熸枻鎷烽敓鐙＄?规嫹閿熸枻鎷烽敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹
        for (int t = 0; t < contours.size(); t++)
        {
            num++;
            vector<Point>point;
            int epsilon = 0.01 * arcLength(contours[t], true);
            approxPolyDP(contours[t], point, epsilon, true);
            if (point.size() == 3)
            {
                switch (i) {
                case 0:printf("<%d>\t<三角形>\t<黑色>\n", num);
                    break;
                case 1:printf("<%d>\t<三角形>\t<红色>\n", num);
                    break;
                case 2:printf("<%d>\t<三角形>\t<绿色>\n", num);
                    break;
                case 3:printf("<%d>\t<三角形>\t<黄色>\n", num);
                    break;
                case 4:printf("<%d>\t<三角形>\t<蓝色>\n", num);
                    break;
                default:break;
                }

            }
            else if (point.size() == 4)
            {
                switch (i) {
                case 0:printf("<%d>\t<正方形>\t<黑色>\n", num);
                    break;
                case 1:printf("<%d>\t<正方形>\t<红色>\n", num);
                    break;
                case 2:printf("<%d>\t<正方形>\t<绿色>\n", num);
                    break;
                case 3:printf("<%d>\t<正方形>\t<黄色>\n", num);
                    break;
                case 4:printf("<%d>\t<正方形>\t<蓝色>\n", num);
                    break;
                default:break;
                }

            }
            else
            {
                switch (i) {
                case 0:printf("<%d>\t<圆形 > \t<黑色>\n", num);
                    break;
                case 1:printf("<%d>\t<圆形 > \t<红色>\n", num);
                    break;
                case 2:printf("<%d>\t<圆形 > \t<绿色>\n", num);
                    break;
                case 3:printf("<%d>\t<圆形 > \t<黄色>\n", num);
                    break;
                case 4:printf("<%d>\t<圆形 > \t<蓝色>\n", num);
                    break;
                default:break;
                }

            }
            drawContours(igs, contours, t, Scalar(0, 0, 255), 5, 8, Mat(), 0, Point());//dst閿熸枻鎷烽敓鏂ゆ嫹閿熼ズ绛规嫹濮嬮敓鏂ゆ嫹
            //Point  loc=contours[1];
            string s = to_string(num);
            //cout<<contours.front()<<endl;
            putText(igs, s, point[0], FONT_HERSHEY_COMPLEX, 2, Scalar(0, 0, 0), 2, 8, 0);
            //cout << "閿熺??纰夋嫹閿熸枻鎷风洰閿熸枻鎷?" << point.size() << endl;
        }
    }
    namedWindow("igs", 0);
    imshow("igs", igs);
    waitKey(0);

    return 0;
}
