#include <stdio.h>
#include "highgui.h"
#include <opencv2\opencv.hpp>
#include <iostream>
#define VERTICAL  0
#define HORIZONTAL 1
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) 
using namespace cv;
void getEnergyMap(Mat src,Mat gradientMap) {
	//��ԭͼ����ת���ɻҶ�ͼ
	Mat grayImage(src.rows,src.cols,CV_8U,Scalar(0));
	cvtColor(src,grayImage,COLOR_RGB2GRAY);
	//imshow("gray",grayImage);
	//waitKey(0);
	Mat gradientH(src.rows,src.cols,CV_32F,Scalar(0));   //Horizontal gradient ˮƽ�ݶȾ���
	Mat gradientV(src.rows,src.cols,CV_32F,Scalar(0));   //Vertical gradient   ��ֱ�ݶȾ���
	//��������filter2D�������о�� Ч�ʲ��Ǻܸ�,���ǽ����ȷ
	Mat kernelH = (Mat_<float>(3,3) << 0,0,0,0,1,-1,0,0,0 );   //����ˮƽ�����ϵľ���˺���
	Mat kernelV = (Mat_<float>(3,3) << 0,0,0,0,1,0,0,-1,0);	   //������ֱ�����ϵľ���˺���
	filter2D(grayImage,gradientH,gradientH.depth(),kernelH);  
	filter2D(grayImage,gradientV,gradientV.depth(),kernelV);  //�ֱ��ˮƽ����ֱ������о��

/*
	for (int j = 0; j < src.cols; j++) {
		gradientV.at<float>(src.rows - 1, j) = 0;
		if (j != src.cols - 1) {
			for (int i = 0; i < src.rows; i++) {
				gradientH.at<float>(i, j) = grayImage.at<char>(i, j) - grayImage.at<char>(i, j + 1);
			}
		}
	}
	for (int i = 0; i < src.rows; i++) {
		gradientH.at<float>(i, src.cols - 1) = 0;
		if (i != src.rows - 1) {
			for (int j = 0; j < src.cols; j++) {
				gradientV.at<float>(i, j) = grayImage.at<char>(i, j) - grayImage.at<char>(i + 1, j);
			}
		}
	}
*/
	add(abs(gradientH),abs(gradientV),gradientMap);    //��ˮƽ����ֱ���˲������� ���Եõ������ݶȴ�С ���ھ���gradientMap
	//Mat showGradient;
	//gradientMap.convertTo(showGradient,CV_8U,1,0);  //����gradientMap�������cv_32f ,Ҫ��ʾ��������ת����cv_8u
	//imshow("gradientShow",showGradient);
	//waitKey(0);     //�����п�����ʾ�����ݶ�ͼ��
}
void calculateEnergy(Mat EnergyMap,Mat Accum,Mat traceMap,int code) {
//ע��traceMap��ÿ�����ص�λ��ֻ��0��1��2������ֵ  ������־�����ص�Ӻδ��̳ж�����
	EnergyMap.copyTo(Accum);						//ʹ��copyTo������= ����ΪҪ��������������������Ǿ���ͷ��
	if (code == 0) {									//��ֱ�����seam  traceMap������Ϊ0 ����Ϊ1 ����Ϊ2
		for (int i = 1; i < EnergyMap.rows; i++) {		//�ӵڶ��п�ʼ����
			//��һ��
			if (Accum.at<float>(i - 1, 0) <= Accum.at<float>(i - 1, 1)) {
				Accum.at<float>(i, 0) = Accum.at<float>(i - 1, 0) + EnergyMap.at<float>(i, 0);
				traceMap.at<char>(i, 0) = 1;
			}
			else {										//���Ϸ����ص����������ұ�һ�е���� ȡ���Ͻǵĵ�
				Accum.at<float>(i, 0) = Accum.at<float>(i - 1, 1) + EnergyMap.at<float>(i, 0);
				traceMap.at<char>(i, 0) = 2;
			}
			//�м���
			for (int j = 1; j < EnergyMap.cols -1; j++) {
				float k[3];
				k[0] = Accum.at<float>(i - 1, j - 1);
				k[1] = Accum.at<float>(i - 1, j);
				k[2] = Accum.at<float>(i - 1, j + 1);
				int index = 0;
				if (k[0] > k[1]) index = 1;
				if (k[2] < k[index]) index = 2;
				Accum.at<float>(i, j) = Accum.at<float>(i - 1, j - 1 + index) + EnergyMap.at<float>(i, j);
				traceMap.at<char>(i, j) = index;
			}
			//���һ�У����ұ�һ�У�
			if (Accum.at<float>(i - 1, EnergyMap.cols - 1) <= Accum.at<float>(i - 1, EnergyMap.cols - 2)) {
				Accum.at<float>(i, EnergyMap.cols - 1) = Accum.at<float>(i - 1, EnergyMap.cols - 1) + EnergyMap.at<float>(i, EnergyMap.cols - 1);
				traceMap.at<char>(i, EnergyMap.cols - 1) = 1;
			}
			else {  //�����Ϸ��̳е����
				Accum.at<float>(i, EnergyMap.cols - 1) = Accum.at<float>(i - 1, EnergyMap.cols - 2) + EnergyMap.at<float>(i, EnergyMap.cols - 1);
				traceMap.at<char>(i, EnergyMap.cols - 1) = 0;
			}
		}
	}
	else if (code == 1) {  //��������Ҽ����ۼ�����  Ѱ��ˮƽ�����seam ����Ϊ0�������Ϊ1������Ϊ2
		for (int i = 1; i < EnergyMap.cols; i++) {	//�ӵڶ��п�ʼ����
			//��һ��
			if (Accum.at<float>(0, i - 1) <= Accum.at<float>(1, i - 1)) {		//��ߵı����µ�С����ȡ��ߵ�
				Accum.at<float>(0, i) = Accum.at<float>(0, i - 1) + EnergyMap.at<float>(0, i);
				traceMap.at<char>(0, i) = 1;
			}
			else {	//���·������ص��С ��ȡ���½�=2
				Accum.at<float>(0, i) = Accum.at<float>(1, i - 1) + EnergyMap.at<float>(0, i);
				traceMap.at<char>(0, i) = 2;
			}
			//�м���
			for (int j = 1; j < EnergyMap.rows - 1; j++) {
				float k[3];
				k[0] = Accum.at<float>(j - 1, i - 1);
				k[1] = Accum.at<float>(j, i - 1);
				k[2] = Accum.at<float>(j + 1, i - 1);
				int index = 0;
				if (k[0] > k[1]) index = 1;
				if (k[2] < k[index]) index = 2;
				Accum.at<float>(j, i) = Accum.at<float>(j - 1 + index, i - 1) + EnergyMap.at<float>(j, i);
				traceMap.at<char>(j, i) = index;
			}
			//������һ��
			int c = EnergyMap.rows - 1;
			if (Accum.at<float>(c - 1, i - 1) <= Accum.at<float>(c, i - 1)) {	//���Ϸ����ص��С
				Accum.at<float>(c, i) = Accum.at<float>(c - 1, i - 1) + EnergyMap.at<float>(c, i);
				traceMap.at<char>(c, i) = 0;
			}
			else {	//�󷽸�С
				Accum.at<float>(c, i) = Accum.at<float>(c, i - 1) + EnergyMap.at<float>(c, i);
				traceMap.at<char>(c, i) = 1;
			}
		}
	}
}
void FindSeam(const Mat Acum,const Mat traceMap,Mat Seam,int code) {
	if (code == 0) {  //�����seam
		int row = Acum.rows - 1;
		int index = 0; //��������б�
		//�����ʶ�����һ���е���Сֵ
		for (int i = 1; i < Acum.cols; i++) {
			if (Acum.at<float>(row, i) < Acum.at<float>(row, index)) index = i;  //Ѱ����С����ֵ�п�����ߵ�
		}
		//�������traceMap���ϻ��ݣ��ҳ�minTrace
		Seam.at<float>(row, 0) = index;
		int pos = index; //��¼��traceMap�е�ǰ��λ�á�
		for (int i = row; i > 0; i--) {
			if (traceMap.at<char>(i, pos) == 0) {		//������
				index -= 1;
			}
			else if (traceMap.at<char>(i, pos) == 2) {		//������
				index += 1;
			}
			Seam.at<float>(i - 1, 0) = index;
			pos = index;
		}
	}
	else if (code == 1) {	//�����seam
		int col = Acum.cols - 1;
		int index = 0; //��¼�б�
		for (int i = 1; i < Acum.rows; i++) {
			if (Acum.at<float>(i, col) < Acum.at<float>(index, col)) index = i;
		}
		//��traceMap�����һ����ǰ���ݣ��ҳ������seam
		Seam.at<float>(0, col) = index;
		int pos = index;
		for (int i = col; i > 0; i--) {
			if (traceMap.at<char>(pos, i) == 0) {		//������
				index -= 1;
			}
			else if (traceMap.at<char>(pos, i) == 2) {		//������
				index += 1;
			}
			Seam.at<float>(0, i - 1) = index;
			pos = index;
		}
	}
}
void ShowSeam(const Mat src,Mat Seam,int code) {
	Mat show(src.rows, src.cols, src.type());
	src.copyTo(show);
	if (code == 0) {	//�ú���չʾ����Seam
		for (int i = 0; i < src.rows; i++) {
			int k = Seam.at<float>(i, 0);
			show.at<Vec3b>(i, k)[0] = 0;		//B
			show.at<Vec3b>(i, k)[1] = 0;		//G
			show.at<Vec3b>(i, k)[2] = 255;		//R
		}		
		imshow("ShowSeam",show);
//		waitKey(0);
	}
	else if (code == 1) {
		for (int i = 0; i < src.cols; i++) {
			int k = Seam.at<float>(0,i);
			show.at<Vec3b>(k, i)[0] = 0;
			show.at<Vec3b>(k, i)[1] = 0;
			show.at<Vec3b>(k, i)[2] = 255;
		}
		imshow("ShowSeam",show);
	}
}
void DeleteOneCol(Mat pre,Mat result,Mat seam) {
	for (int i = 0; i < pre.rows; i++) {
		int k = seam.at<float>(i, 0);
		for (int j = 0; j < k; j++) {			//seam��ߵ����ص㲻��
			result.at<Vec3b>(i, j)[0] = pre.at<Vec3b>(i, j)[0];
			result.at<Vec3b>(i, j)[1] = pre.at<Vec3b>(i, j)[1];
			result.at<Vec3b>(i, j)[2] = pre.at<Vec3b>(i, j)[2];
		}
		for (int j = k; j < pre.cols - 1; j++) {
			if (j == pre.cols - 1) continue;  //���j=k=ԭͼ���һ���ϵ����ص㣬��ô������ߵ����ص㶼�Ѿ�λ��ֱ�ӽ�����һ�С�
			result.at<Vec3b>(i, j)[0] = pre.at<Vec3b>(i, j + 1)[0];
			result.at<Vec3b>(i, j)[1] = pre.at<Vec3b>(i, j + 1)[1];
			result.at<Vec3b>(i, j)[2] = pre.at<Vec3b>(i, j + 1)[2];
		}
	}
}
void DeleteOneRow(Mat pre,Mat result,Mat seam) {
	for (int i = 0; i < pre.cols; i++) {
		int k = seam.at<float>(0, i);	
		for (int j = 0; j < k; j++) {	//seam��������ص㲻��
			result.at<Vec3b>(j, i)[0] = pre.at<Vec3b>(j, i)[0];
			result.at<Vec3b>(j, i)[1] = pre.at<Vec3b>(j, i)[1];
			result.at<Vec3b>(j, i)[2] = pre.at<Vec3b>(j, i)[2];
		}
		for (int j = k; j < pre.rows - 1; j++) {		//seam�·��ĵ�ͳһ����һ����λ
			if (j == pre.rows - 1) continue;
			result.at<Vec3b>(j, i)[0] = pre.at<Vec3b>(j + 1, i)[0];
			result.at<Vec3b>(j, i)[1] = pre.at<Vec3b>(j + 1, i)[1];
			result.at<Vec3b>(j, i)[2] = pre.at<Vec3b>(j + 1, i)[2];
		}
	}
}

void run(Mat src, Mat result,int dirCode) {
	Mat gradientMap(src.rows, src.cols, CV_32F, Scalar(0));
	getEnergyMap(src, gradientMap);    //����Ҫ�Ĵ�ÿ�����ص������ľ��󣬾���gradientMap.
	Mat energyAccumulation(src.rows, src.cols, CV_32F, Scalar(0));   //�½��������ۻ�����
	Mat traceMap(src.rows, src.cols, CV_8UC1, Scalar(0));		 //�½���·����¼����
	calculateEnergy(gradientMap, energyAccumulation, traceMap, dirCode); //ע�� VERTICAL��HORIZONTAL����ָҪѰ��seam������
	//����directionCode  �½�һ��װseam��Mat������ 
	Mat Seam[2];
	Mat Vseam(src.rows,1,CV_32F,Scalar(0));
	Mat Hseam(1, src.cols, CV_32F, Scalar(0)); 
	Seam[0] = Vseam;
	Seam[1] = Hseam;
	FindSeam(energyAccumulation,traceMap,Seam[dirCode],dirCode);
//	ShowSeam(src,Seam[dirCode],dirCode);

	if (dirCode == 0) {  //ɾ����һ��
		DeleteOneCol(src,result,Seam[dirCode]);
	}
	else if (dirCode == 1) {   //ɾ����һ�� 
		DeleteOneRow(src, result, Seam[dirCode]);
	}

}

int main(int argc, char* argv) {
	Mat original = imread("E:\\programming\\MATLAB\\pictures\\Beckham01.jpg");
	namedWindow("original");
	imshow("original",original);
	//waitKey(0);
	Mat temp;
	original.copyTo(temp);
	int linesToBeDeleted = 100;

	//�����խ
	Mat finalImage(original.rows,original.cols-linesToBeDeleted,original.type());
	for (int i = 0; i < linesToBeDeleted; i++) {
		Mat tempResult(temp.rows, temp.cols - 1, temp.type());
		run(temp, tempResult, VERTICAL);		//ע�� VERTICAL��HORIZONTAL����ָҪѰ��seam������
		temp = tempResult;
		if (i == linesToBeDeleted - 1)	tempResult.copyTo(finalImage);
	}

	//�߶ȱ䰫
/*	Mat finalImage(original.rows - linesToBeDeleted, original.cols, original.type());
	for (int i = 0; i < linesToBeDeleted; i++) {
		Mat tempResult(temp.rows-1,temp.cols,temp.type());
		run(temp, tempResult, HORIZONTAL);
		temp = tempResult;
		if (i == linesToBeDeleted - 1) tempResult.copyTo(finalImage);
	}
*/
//	Mat finalImage(original.rows,original.cols - 1,original.type());		//����
//	Mat finalImage(original.rows - 1, original.cols, original.type());		//����
//	run(temp,finalImage,VERTICAL);   //���Ҫ��ʾseam ���������forѭ��ע�͵�������run()��ShowSeam���µĲ���ע�͵�
	imshow("final",finalImage);
	waitKey(0);
	
}

