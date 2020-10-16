#include"cornerPoint.h"

void innter() //角点检测
{
	Mat imme = imread("E://photo_opencv/girl.jpg", 0);
	Mat src = imread("E://photo_opencv/girl.jpg", 1);
	Mat dest, dst;
	Mat getout;
	int thresh = 80;
	imwrite("gray.jpg", imme);
	cornerHarris(imme, dest, 2, 3, 0.04);
	imwrite("corePoint.jpg", dest);
	threshold(dest, dst, 0.00001, 255, THRESH_BINARY);
	imwrite("THre.jpg", dst);
	normalize(dst, dst, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
	convertScaleAbs(dst, getout);//将归一化后的图线性变换成8位无符号整型   

								 //---------------------------【4】进行绘制-------------------------------------  
								 // 将检测到的，且符合阈值条件的角点绘制出来  

	for (int j = 0; j < dst.rows; j++)
	{
		for (int i = 0; i < dst.cols; i++)
		{
			if ((int)dst.at<float>(j, i) > thresh + 80)
			{
				circle(getout, Point(i, j), 5, Scalar(10, 10, 255), 2, 8, 0);
				circle(src, Point(i, j), 5, Scalar(10, 10, 255), 2, 8, 0);
			}
		}
	}

	imwrite("imgpoint.jpg", getout);
	imwrite("imgpoin_srct.jpg", src);
}


void SUfrte() //SUFR检测   不知道哪里出错了
{
	Mat src = imread("E://photo_opencv/girl.jpg", 1);
	Mat imme = imread("E://photo_opencv/girl.jpg", 1);
	Mat out, dst;
	vector<KeyPoint>  namepoint, eother;
	Mat descriptor1, descriptor2;
	Ptr<Feature2D> surf;
	surf = xfeatures2d::SURF::create(4000);
	surf->detectAndCompute(src, Mat(), namepoint, descriptor1);
	surf->detectAndCompute(imme, Mat(), eother, descriptor2);

	BFMatcher matcher;
	vector<DMatch> matches;
	matcher.match(descriptor1, descriptor2, matches);
	//sort(matches.begin(), matches.end());     //筛选匹配点，根据match里面特征对的距离从小到大排序  
	//vector< DMatch > good_matches;
	//	int ptsPairs = std::min(50, (int)(matches.size() * 0.15));
	//cout << ptsPairs << endl;
	//for (int i = 0; i < ptsPairs; i++)
	//{
	//	good_matches.push_back(matches[i]);//距离最小的50个压入新的DMatch  
	//	}
	Mat matchimg;
	drawMatches(descriptor1, namepoint, descriptor2, eother, matches, matchimg);
	/*imwrite("detectAnd.jpg", descriptor1);

	drawKeypoints(src, namepoint, out,Scalar(20,20,150));
	cout << "namepoint.size() = " << namepoint.size() << endl;

	Mat matchimg;
	FlannBasedMatcher matcher;
	vector<DMatch>matches;
	matcher.match(descriptor1, descriptor2, matches);

	double max_dist = 0;
	double min_dist = 100;
	for (int i = 0; i < descriptor1.rows; i++)
	{
	double dist = matches[i].distance;
	if (dist < min_dist)min_dist = dist;
	if (dist > max_dist)max_dist = dist;
	}

	vector<DMatch>good_matches;
	for (int i = 0; i < descriptor1.rows; i++)
	{
	if (matches[i].distance <= max(2 * min_dist, 0.02))
	{
	good_matches.push_back(matches[i]);
	}
	}*/

	//drawMatches(descriptor1, namepoint, descriptor2, eother, good_matches, matchimg);

	//	for (int i = 0; i < namepoint.size();i++)
	//	{
	//	cout << " i =  " << i << "\tnamepoint[i].pt.x = " << namepoint[i].pt.x << "\tnamepoint[i].pt.y =  " << namepoint[i].pt.y << endl;
	//}
	imwrite("matchimg.jpg", matchimg);
	//imwrite("imgsurf_40.jpg",out);
}


//这一种方法是可以的
void whatSuccess()
{
	Mat srcImage1 = imread("E://photo_opencv/girl.jpg", 1);
	Mat srcImage2 = imread("E://photo_opencv/girl.jpg", 1);


	Ptr<xfeatures2d::SurfFeatureDetector> detector = xfeatures2d::SurfFeatureDetector::create(4000);

	vector<KeyPoint> key_points_1, key_points_2;

	Mat dstImage1, dstImage2;
	detector->detectAndCompute(srcImage1, Mat(), key_points_1, dstImage1);
	detector->detectAndCompute(srcImage2, Mat(), key_points_2, dstImage2);//可以分成detect和compute

	imwrite("Detecor_1.jpg", dstImage1);
	imwrite("Detecor_2.jpg", dstImage2);

	Mat img_keypoints_1, img_keypoints_2;
	drawKeypoints(srcImage1, key_points_1, img_keypoints_1, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
	drawKeypoints(srcImage2, key_points_2, img_keypoints_2, Scalar::all(-1), DrawMatchesFlags::DEFAULT);


	imwrite("Sucee_1.jpg", img_keypoints_1);
	imwrite("Sucee_2.jpg", img_keypoints_2);

	//  进行BFMatch暴力匹配
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");
	vector<DMatch>mach;

	matcher->match(dstImage1, dstImage2, mach);
	/*double Max_dist = 0;
	double Min_dist = 100;
	for (int i = 0; i < dstImage1.rows; i++)
	{
	double dist = mach[i].distance;
	if (dist < Min_dist)Min_dist = dist;
	if (dist > Max_dist)Max_dist = dist;
	}
	cout << "最短距离" << Min_dist << endl;
	cout << "最长距离" << Max_dist << endl;

	vector<DMatch>goodmaches;
	for (int i = 0; i < dstImage1.rows; i++)
	{
	if (mach[i].distance < 2 * Min_dist)
	goodmaches.push_back(mach[i]);
	}*/
	Mat img_maches;
	drawMatches(srcImage1, key_points_1, srcImage2, key_points_2, mach, img_maches); //goodmaches

																					 //for (int i = 0; i < goodmaches.size(); i++)
																					 //{
																					 //	cout << "符合条件的匹配：" << goodmaches[i].queryIdx << "--" << goodmaches[i].trainIdx << endl;
																					 //}
	imwrite("Finaldest.jpg", img_maches);

}


// 这个也可以 基于SURF的检测
void  another()
{
	Mat objImage = imread("E://photo_opencv/girl.jpg", IMREAD_COLOR);
	Mat sceneImage = imread("E://photo_opencv/girl.jpg", IMREAD_COLOR);
	//-- Step 1: Detect the keypoints using SURF Detector
	int minHessian = 400;
	Ptr<xfeatures2d::SURF> detector = xfeatures2d::SURF::create(minHessian);
	std::vector<KeyPoint> obj_keypoint, scene_keypoint;
	detector->detect(objImage, obj_keypoint);
	detector->detect(sceneImage, scene_keypoint);
	//computer the descriptors
	Mat obj_descriptors, scene_descriptors;
	detector->compute(objImage, obj_keypoint, obj_descriptors);
	detector->compute(sceneImage, scene_keypoint, scene_descriptors);
	//use BruteForce to match,and get good_matches
	BFMatcher matcher;
	vector<DMatch> matches;
	matcher.match(obj_descriptors, scene_descriptors, matches);
	sort(matches.begin(), matches.end());  //筛选匹配点
	vector<DMatch> good_matches;
	for (int i = 0; i < min(50, (int)(matches.size()*0.15)); i++) {
		good_matches.push_back(matches[i]);
	}
	//draw matches
	Mat imgMatches;
	drawMatches(objImage, obj_keypoint, sceneImage, scene_keypoint, good_matches, imgMatches);

	imwrite("donotknow.jpg", imgMatches);

}

void ItisMyTest()
{
	Mat imme = imread("E://photo_opencv/girl.jpg", 1);
	Mat imme_2 = imread("E://photo_opencv/girl_10.jpg", 1);
	Ptr<xfeatures2d::SURF> detector = xfeatures2d::SURF::create(400);
	vector<KeyPoint> obj_keypoint, scene_keypoint;
	Mat aa, bb;
	detector->detectAndCompute(imme, Mat(), obj_keypoint, aa);
	detector->detectAndCompute(imme_2, Mat(), scene_keypoint, bb);
	FlannBasedMatcher matcher;
	vector<DMatch>  matches;
	matcher.match(aa, bb, matches);

	double min_dst = 2000;
	double max_dst = 0;

	for (int i = 0; i < matches.size(); i++)
	{
		double dist = matches[i].distance;
		if (dist < min_dst)min_dst = dist;
		if (dist > max_dst)max_dst = dist;
	}
	cout << "min_dst  =   " << min_dst << "      max_dst  =   " << max_dst << endl;
	cout << "matches.size() = " << matches.size() << endl;
	for (int nn = 0; nn < 50; nn++)
	{
		cout << "i =   " << nn << "    matches[i].distance  =   " << matches[nn].distance << endl;
	}
	vector<DMatch>good_matches;

	for (int i = 0; i < matches.size(); i++)
	{
		if (matches[i].distance <= max((double)(4 * min_dst), 0.02))
		{
			good_matches.push_back(matches[i]);
		}
	}
	//这个筛选不好结果
	//sort(matches.begin(), matches.end());  //筛选匹配点
	//vector<DMatch> good_matches;
	//for (int i = 0; i < min(50, (int)(matches.size()*0.15)); i++) {
	//	good_matches.push_back(matches[i]);
	//}
	Mat destimg;
	drawMatches(imme, obj_keypoint, imme_2, scene_keypoint, good_matches, destimg);
	imwrite("SURFImg.jpg", destimg);
}

void SIftjiance()
{
	Mat imme = imread("E://photo_opencv/girl.jpg", 1);
	Mat imme_2 = imread("E://photo_opencv/girl_10.jpg", 1);
	Ptr<xfeatures2d::SIFT> detector = xfeatures2d::SIFT::create(400);
	vector<KeyPoint> obj_keypoint, scene_keypoint;
	Mat aa, bb;
	detector->detectAndCompute(imme, Mat(), obj_keypoint, aa);
	detector->detectAndCompute(imme_2, Mat(), scene_keypoint, bb);
	FlannBasedMatcher matcher;
	vector<DMatch>  matches;
	matcher.match(aa, bb, matches);


	Mat destimg;
	drawMatches(imme, obj_keypoint, imme_2, scene_keypoint, matches, destimg);
	imwrite("SIfttImg.jpg", destimg);
}



//getRotationMatrix2D 用于仿射检测 这个以一个点为变化
void Wrapchange()
{
	Mat objImage = imread("E://photo_opencv/girl.jpg", 1);
	Point center;
	cout << "\t objImage.rows =  " << objImage.rows << "\t objImage.cols" << objImage.cols << endl;
	center.x = objImage.cols / 2;
	center.y = objImage.rows / 2;
	float angle = 30.0;
	float scal = 1.0;
	Mat img = getRotationMatrix2D(center, angle, scal);
	Mat dest, dst;
	warpAffine(objImage, dest, img, objImage.size());
	imwrite("Pointchange.jpg", dest);

}

void WrapFourPoint()
{
	Mat imme = imread("E://photo_opencv/girl.jpg", 1);
	Mat dest = Mat(imme.size(), imme.type());
	Point2f src[3];
	src[0] = Point(0, 0);
	src[1] = Point(static_cast<int>(imme.rows - 1), 0);
	src[2] = Point(0, static_cast<int>(imme.cols - 1));

	Point2f dats[3];
	dats[0] = Point(0, 0);
	dats[1] = Point((int)(imme.rows - 1)*0.11, (int)(imme.cols - 1)*0.33);
	dats[2] = Point((int)(imme.rows - 1)*0.33, (int)(imme.cols - 1)*0.65);

	Mat destimg;
	destimg = getAffineTransform(src, dats);
	warpAffine(imme, dest, destimg, imme.size());
	imwrite("whatitlike.jpg", dest);
}