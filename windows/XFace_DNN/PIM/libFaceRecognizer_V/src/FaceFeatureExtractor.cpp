#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#include "libFaceRecognizer_V/inc/FaceFeatureExtractor.h"
#include "libFaceRecognizer_V/inc/MeanFeature.h"
#include "libPlatform/inc/ImageProcess.h"

#include "libFaceRecognizer_V/inc/FR_Params.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "boost/algorithm/string.hpp"
#include "boost/smart_ptr/shared_ptr.hpp"

#include "caffe/caffe.hpp"
#include "caffe/util/math_functions.hpp"
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#define _OPENMP

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace caffe;
using namespace boost;
using std::string;


void CFaceFeatureExtractor::WrapInputLayer(int net_type, std::vector<cv::Mat>* input_channels)
{
	Blob<float>* input_layer;

	if (net_type == 951)
		input_layer = net_95_1->input_blobs()[0];
	else if (net_type == 952)
		input_layer = net_95_2->input_blobs()[0];
	else if (net_type == 67)
		input_layer = net_67->input_blobs()[0];
	else if (net_type == 47)
		input_layer = net_47->input_blobs()[0];
	else
		return;

	int width = input_layer->width();
	int height = input_layer->height();
	float* input_data = input_layer->mutable_cpu_data();
	for (int i = 0; i < input_layer->channels(); ++i)
	{
		cv::Mat channel(height, width, CV_32FC1, input_data);
		input_channels->push_back(channel);
		input_data += width * height;
	}
}

PIM_Bool CFaceFeatureExtractor::image_transform(PIM_Bitmap *pBitmap, std::vector<cv::Mat>* input_channels)
{
	int img_size = pBitmap->width*pBitmap->height * 3;

	cv::Mat sample;
	sample.create(pBitmap->height, pBitmap->width, CV_8UC3);
	memcpy(sample.data, pBitmap->imageData, sizeof(PIM_Uint8)*pBitmap->width*pBitmap->height * 3);

	cv::Mat sample_float;
	sample.convertTo(sample_float, CV_32FC3, 0.00390625);

	//sample_float *= 0.00390625;

	cv::split(sample_float, *input_channels);

	sample.release();
	sample_float.release();
	//sample_normalized.release();

	return PIM_TRUE;
}


CFaceFeatureExtractor::CFaceFeatureExtractor()
{
	// Read the resources from dat files
	FILE* fp = NULL;
	fp = fopen(DNN_TRANS_MAT_PATH, "rb");
	if (fp == NULL)
	{
		printf("error open files \n");
		return;
	}

	trans_sqrtlbp = (PIM_Float *)malloc(FR_FEATURE_DNN_ENSEMBLE * PCA_FEATURE_DIM * sizeof(PIM_Float));
	fread(trans_sqrtlbp, sizeof(PIM_Float), FR_FEATURE_DNN_ENSEMBLE * PCA_FEATURE_DIM, fp);
	fclose(fp);

	char szT = 'P';
	char *psz = &szT;
	char **szName = &psz;

	int nN = 1;

	caffe::GlobalInit(&nN, &szName);


#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	/*for (int gid = 0; gid < 4; gid++)
	{
	Caffe::SetDevice(gid);
	Caffe::DeviceQuery();
	}*/
	Caffe::SetDevice(0);
	Caffe::set_mode(Caffe::GPU);
#endif

	string modelfile1(MODEL_NET_95_1);

	net_95_1.reset(new Net<float>(modelfile1, caffe::TEST));
	net_95_1->CopyTrainedLayersFrom(MODEL_95_1);

	net_95_2.reset(new Net<float>(MODEL_NET_95_2, TEST));
	net_95_2->CopyTrainedLayersFrom(MODEL_95_2);

	net_67.reset(new Net<float>(MODEL_NET_67, TEST));
	net_67->CopyTrainedLayersFrom(MODEL_67);

	net_47.reset(new Net<float>(MODEL_NET_47, TEST));
	net_47->CopyTrainedLayersFrom(MODEL_47);

}

CFaceFeatureExtractor::~CFaceFeatureExtractor()
{
	free(trans_sqrtlbp);
}


PIM_Int32 CFaceFeatureExtractor::FeatureExtract(PIM_Bitmap *pBitmap_95, PIM_Bitmap *pBitmap_67, PIM_Bitmap *pBitmap_47, PIM_Float *feature)
{
	if (pBitmap_95->imageData == NULL || pBitmap_67->imageData == NULL || pBitmap_47->imageData == NULL)
	{
		printf("FeatureExtract: Input image is null.\n");
		return PIM_ERROR_GENERAL;
	}
// 	Timer timer_1, timer_2, timer_3, timer_4;
// 	Timer total_timer;
// 	Timer projection_timer;

	//total_timer.Start();

	float temp_feature[8192];
	memset(temp_feature, 0, sizeof(float)* 8192);

	// 95x95_1 feature extraction
	Blob<float>* input_layer = net_95_1->input_blobs()[0];
	input_layer->Reshape(1, 3, pBitmap_95->height, pBitmap_95->width);
	net_95_1->Reshape();

	std::vector<cv::Mat> input_channels;
	WrapInputLayer(951, &input_channels);

	image_transform(pBitmap_95, &input_channels);

	//timer_1.Start();
	//net_95_1->ForwardPrefilled();
	net_95_1->Forward();
	//timer_1.Stop();

	const boost::shared_ptr<Blob<float>> feature1_blob = net_95_1->blob_by_name("cls_4a_fc1_bn");
	const boost::shared_ptr<Blob<float>> feature2_blob = net_95_1->blob_by_name("cls_4c_fc1_bn");

	memcpy(temp_feature, feature1_blob->cpu_data(), sizeof(float)* 1024);
	memcpy(temp_feature + 1024, feature2_blob->cpu_data(), sizeof(float)* 1024);


	// 95x95_2 feature extraction
	input_layer = net_95_2->input_blobs()[0];
	input_layer->Reshape(1, 3, pBitmap_95->height, pBitmap_95->width);
	net_95_2->Reshape();

	WrapInputLayer(952, &input_channels);
	image_transform(pBitmap_95, &input_channels);


	//timer_2.Start();
	//net_95_2->ForwardPrefilled();
	net_95_2->Forward();
	//timer_2.Stop();

	const boost::shared_ptr<Blob<float>> feature3_blob = net_95_2->blob_by_name("cls_4a_fc1_bn");
	const boost::shared_ptr<Blob<float>> feature4_blob = net_95_2->blob_by_name("cls_4c_fc1_bn");

	memcpy(temp_feature + 2048, feature3_blob->cpu_data(), sizeof(float)* 1024);
	memcpy(temp_feature + 3072, feature4_blob->cpu_data(), sizeof(float)* 1024);


	// 67x67 feature extraction
	input_layer = net_67->input_blobs()[0];
	input_layer->Reshape(1, 3, pBitmap_67->height, pBitmap_67->width);
	net_67->Reshape();

	WrapInputLayer(67, &input_channels);
	image_transform(pBitmap_67, &input_channels);

	//timer_3.Start();
	//net_67->ForwardPrefilled();
	net_67->Forward();
	//timer_3.Stop();

	const boost::shared_ptr<Blob<float>> feature5_blob = net_67->blob_by_name("cls_4a_fc1_bn");
	const boost::shared_ptr<Blob<float>> feature6_blob = net_67->blob_by_name("cls_4c_fc1_bn");

	memcpy(temp_feature + 4096, feature5_blob->cpu_data(), sizeof(float)* 1024);
	memcpy(temp_feature + 5120, feature6_blob->cpu_data(), sizeof(float)* 1024);


	// 47x47 feature extraction
	input_layer = net_67->input_blobs()[0];
	input_layer->Reshape(1, 3, pBitmap_47->height, pBitmap_47->width);
	net_47->Reshape();

	WrapInputLayer(47, &input_channels);
	image_transform(pBitmap_47, &input_channels);

	//timer_4.Start();
	//net_47->ForwardPrefilled();
	net_47->Forward();
	//timer_4.Stop();

	const boost::shared_ptr<Blob<float>> feature7_blob = net_47->blob_by_name("cls_4a_fc1_bn");
	const boost::shared_ptr<Blob<float>> feature8_blob = net_47->blob_by_name("cls_4c_fc1_bn");

	memcpy(temp_feature + 6144, feature7_blob->cpu_data(), sizeof(float)* 1024);
	memcpy(temp_feature + 7168, feature8_blob->cpu_data(), sizeof(float)* 1024);

#if 1
	//projection_timer.Start();
	PCA_Projection(temp_feature, feature);
	//projection_timer.Stop();
#endif

// 	total_timer.Stop();
// 	printf("95x95_1 processing: %lf ms.\n", timer_1.MilliSeconds());
// 	printf("95x95_2 processing: %lf ms.\n", timer_2.MilliSeconds());
// 	printf("67x67 processing: %lf ms.\n", timer_3.MilliSeconds());
// 	printf("47x47 processing: %lf ms.\n", timer_4.MilliSeconds());
// 	printf("Projection processing: %lf ms.\n", projection_timer.MilliSeconds());
// 	printf("Total proceesing time: %lf ms.\n", total_timer.MilliSeconds());

	return PIM_SUCCESS;
}



PIM_Bool CFaceFeatureExtractor::PCA_Projection(float *src_feature, float *des_feature)
{
	PIM_Float fData[FR_FEATURE_DNN_ENSEMBLE];

	for (int j = 0; j < FR_FEATURE_DNN_ENSEMBLE; j++)
	{
		fData[j] = ((float)src_feature[j]) - mean_f_dnn[j];
	}

	PIM_Float *pProjVec = trans_sqrtlbp;
	PIM_Float *pFeature = des_feature;
	PIM_Float p = 0.0f;
	int i = 0;
	int j = 0;
	PIM_Float *pULBPFeaturVec;

	for (i = 0; i < PCA_FEATURE_DIM; i++)
	{
		pULBPFeaturVec = fData;
		//*pFeature = 0;
		pFeature[i] = 0;

		for (j = 0; j < FR_FEATURE_DNN_ENSEMBLE; j++)
		{
			//*pFeature += *pULBPFeaturVec++ * *pProjVec++;
			pFeature[i] += *pULBPFeaturVec++ * *pProjVec++;
		}
		//pFeature++;
	}

	return PIM_TRUE;
}