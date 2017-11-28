
#ifndef _FACE_FEATURE_EXTRACTOR_H_
#define _FACE_FEATURE_EXTRACTOR_H_

#pragma once

#include "exp/PIMTypes.h"


//#include "libFaceRecognizer_V/inc/FR_Params.h"

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "caffe/proto/caffe.pb.h"
// #include "gflags/gflags.h"
// #include "glog/logging.h"

#include "boost/algorithm/string.hpp"
#include "boost/smart_ptr/shared_ptr.hpp"
#include "google/protobuf/text_format.h"

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/net.hpp"

/*#include "caffe/caffe.hpp"*/

using namespace caffe;
using namespace boost;

#ifdef __cplusplus
extern "C" {
#endif


	class CFaceFeatureExtractor
	{
	public:
		CFaceFeatureExtractor();
		~CFaceFeatureExtractor();

		PIM_Int32 FeatureExtract(PIM_Bitmap *pBitmap_95, PIM_Bitmap *pBitmap_67, PIM_Bitmap *pBitmap_47, PIM_Float *feature);
	private:
		void WrapInputLayer(int net_type, std::vector<cv::Mat>* input_channels);
		PIM_Bool image_transform(PIM_Bitmap *pBitmap, std::vector<cv::Mat>* input_channels);
		PIM_Bool PCA_Projection(float *src_feature, float *des_feature);

		boost::shared_ptr<Net<float>> net_95_1;
		boost::shared_ptr<Net<float>> net_95_2;
		boost::shared_ptr<Net<float>> net_67;
		boost::shared_ptr<Net<float>> net_47;

		PIM_Float *trans_sqrtlbp;
	};


#ifdef __cplusplus
}
#endif

#endif // #ifndef __FACE_FEATURE_EXTRACTOR_H__