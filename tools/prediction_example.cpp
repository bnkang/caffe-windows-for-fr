#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>
#include <assert.h>
#include "caffe/caffe.hpp"
#include "caffe/util/io.hpp"
#include "caffe/blob.hpp"

using namespace caffe;
using namespace std;

#include <windows.h>
class HighResolutionTimer
{
public:
	HighResolutionTimer() { QueryPerformanceFrequency(&Frequency); }
	
	void Reset()
	{
		QueryPerformanceCounter(&StartingTime);
	}
	double GetMs()
	{
		QueryPerformanceCounter(&EndingTime);
		return (EndingTime.QuadPart - StartingTime.QuadPart) * 1000.0 / Frequency.QuadPart;
	}

protected:
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;

};


int main(int argc, char** argv)
{
	//google::InitGoogleLogging("GLOG_minloglevel=2");
	//google::ShutdownGoogleLogging();

	HighResolutionTimer hrt;

	LOG(INFO) << argv[0] << " [GPU] [Device ID]";

    //Setting CPU or GPU
	if (argc >= 2 && strcmp(argv[1], "GPU") == 0) 
	{
		Caffe::set_mode(Caffe::GPU);
		int device_id = 0;
		if (argc == 3) 
        {
      		device_id = atoi(argv[2]);
    	}
		Caffe::SetDevice(device_id);
   		LOG(INFO) << "Using GPU #" << device_id;
	} 
	else 
	{
    	LOG(INFO) << "Using CPU";
    	Caffe::set_mode(Caffe::CPU);
	}
	
	// Load net
	// Assume you are in Caffe master directory
	hrt.Reset();
	Net<float> net("../../examples/prediction_example/prediction_example.prototxt",caffe::TEST);
	printf("%s Time : %f\n", "load net", hrt.GetMs());

	// Load pre-trained net (binary proto)
	// Assume you are already trained the cifar10 example.
	hrt.Reset();
	net.CopyTrainedLayersFrom("../../examples/cifar10/cifar10_quick_iter_5000.caffemodel");
	printf("%s Time : %f\n", "load pre-net", hrt.GetMs());

	Datum datum;
	if (!(ReadImageToDatum("../../examples/images/cat10.png", 1, &datum)))
	{
		printf("reading error\n");
		return 0;
	}

	Blob<float>* blob = new Blob<float>(1, datum.channels(), datum.height(), datum.width());
	
	BlobProto blob_proto;
	blob_proto.set_num(1);
	blob_proto.set_channels(datum.channels());
	blob_proto.set_height(datum.height());
	blob_proto.set_width(datum.width());
	const int data_size = datum.channels() * datum.height() * datum.width();
	int size_in_datum = std::max<int>(datum.data().size(), datum.float_data_size());
	for (int i = 0; i < size_in_datum; ++i) {
		blob_proto.add_data(0.);
	}
	const string& data = datum.data();
	if (data.size() != 0) {
		for (int i = 0; i < size_in_datum; ++i) {
			blob_proto.set_data(i, blob_proto.data(i) + (uint8_t)data[i]);
		}
	}

	//set data into blob
	blob->FromProto(blob_proto);

	//fill the vector
	vector<Blob<float>*> bottom;
	bottom.push_back(blob);
	float type = 0.0;
	
	const vector<Blob<float>*>& result = net.Forward(bottom, &type);
	printf("reading error4\n");

	printf("-------------\n");
	printf(" prediction :  \n");

	// Get probabilities
	const boost::shared_ptr<Blob<float> >& probLayer = net.blob_by_name("prob");
	const float* probs_out = probLayer->cpu_data();

	// Get argmax results
	const boost::shared_ptr<Blob<float> >& argmaxLayer = net.blob_by_name("argmax");

	// Display results
	printf("---------------------------------------------------------------\n");
	const float* argmaxs = argmaxLayer->cpu_data();
	for (int i = 0; i < argmaxLayer->num(); i++)
	{
		printf("Pattern: %d, %d, %f\n", i, argmaxs[i*argmaxLayer->height() + 0], probs_out[i*probLayer->height() + 0]);
	}
	printf("-------------\n");
	
	
	/*
	//Here I can use the argmax layer, but for now I do a simple for :)
	float max = 0;
	float max_i = 0;
	for (int i = 0; i < 1000; ++i) {
		float value = result[0]->cpu_data()[i];
		if (max < value){
			max = value;
			max_i = i;
		}
	}
	LOG(ERROR) << "max: " << max << " i " << max_i;
	*/

	/*
	float loss = 0.0;
	vector<Blob<float>*> results = net.ForwardPrefilled(&loss);
	printf("Result size: %d, Blob size: %d\n", results.size(), net.input_blobs().size());
	printf("%s Time : %f\n", "load net", hrt.GetMs());


	printf("-------------\n");
	printf(" prediction :  \n");

	// Get probabilities
	const boost::shared_ptr<Blob<float> >& probLayer = net.blob_by_name("prob");
	const float* probs_out = probLayer->cpu_data();
	
	// Get argmax results
	const boost::shared_ptr<Blob<float> >& argmaxLayer = net.blob_by_name("argmax");
	
	// Display results
	printf("---------------------------------------------------------------\n");
	const float* argmaxs = argmaxLayer->cpu_data();
	for (int i = 0; i < argmaxLayer->num(); i++) 
	{
		printf("Pattern: %d, %d, %f\n", i, argmaxs[i*argmaxLayer->height() + 0], probs_out[i*probLayer->height() + 0]);
	}
	printf("-------------\n");
	*/


	return 0;
}
