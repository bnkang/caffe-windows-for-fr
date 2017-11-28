#ifndef CAFFE_TRANSFORMER_LAYER_HPP_
#define CAFFE_TRANSFORMER_LAYER_HPP_

#include <string>
#include <utility>
#include <vector>

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/layer.hpp"
#include "caffe/proto/caffe.pb.h"


namespace caffe {

	/**
	* @brief Apply an affine transform on the input layer, where the affine parameters
	*        are learned by back-propagation.
	*        Max Jaderberg etc, Spatial Transformer Networks.
	*
	* TODO(dox): thorough documentation for Forward, Backward, and proto params.
	*/
	template <typename Dtype>
	class TransformerLayer : public Layer<Dtype> 
	{
	public:
		explicit TransformerLayer(const LayerParameter& param) : Layer<Dtype>(param) {}
		virtual void Reshape(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top);

		virtual inline const char* type() const { return "Transformer"; }
		virtual inline int ExactNumBottomBlobs() const { return 2; }
		virtual inline int MinTopBlobs() const { return 1; }

	protected:
		virtual void Forward_cpu(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top);
		virtual void Forward_gpu(const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top);
		virtual void Backward_cpu(const vector<Blob<Dtype>*>& top, const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);
		virtual void Backward_gpu(const vector<Blob<Dtype>*>& top, const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);

		int count_;
		Blob<Dtype> CoordinateTarget;//3*(nwh)
		Blob<Dtype> CoordinateSource;//2*(nwh)
		Blob<Dtype> InterpolateWeight;//4*(nwh)
	};
}

#endif // CAFFE_TRANSFORMER_LAYER_HPP_