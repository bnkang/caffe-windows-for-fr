// This program converts a set of images to a lmdb/leveldb by storing them
// as Datum proto buffers.
// Usage:
//   convert_imageset [FLAGS] ROOTFOLDER/ LISTFILE DB_NAME
//
// where ROOTFOLDER is the root folder that holds all the images, and LISTFILE
// should be a list of files as well as their labels, in the format as
//   subfolder1/file1.JPEG 7
//   ....

#include <algorithm>
#include <fstream>  // NOLINT(readability/streams)
#include <string>
#include <utility>
#include <vector>

#include "boost/scoped_ptr.hpp"
#include "gflags/gflags.h"
#include "glog/logging.h"

#include "caffe/proto/caffe.pb.h"
#include "caffe/util/db.hpp"
#include "caffe/util/io.hpp"
#include "caffe/util/rng.hpp"

using namespace caffe;  // NOLINT(build/namespaces)
using std::pair;
using boost::scoped_ptr;

DEFINE_bool(gray, false, "When this option is on, treat images as grayscale ones");
DEFINE_bool(shuffle, false, "Randomly shuffle the order of images and their labels");
DEFINE_string(backend, "leveldb", "The backend {lmdb, leveldb} for storing the result");
DEFINE_int32(resize_width, 0, "Width images are resized to");
DEFINE_int32(resize_height, 0, "Height images are resized to");
DEFINE_bool(check_size, true, "When this option is on, check that all the datum have the same size");
DEFINE_bool(encoded, false, "When this option is on, the encoded image will be save in datum");
DEFINE_string(encode_type, "", "Optional: What type should we encode the image as ('png','jpg',...).");


typedef struct _triplet_s
{
	std::string anchor_file_name;
	std::string positive_file_name;
	std::string negative_file_name;	
	
	int anchor_label;	
	int positive_label;	
	int negative_label;
} triplet_s;

typedef struct _triplet_lfw_s
{
	std::string anchor_file_name;
	std::string positive_file_name;
	std::string negative_file_name;
	std::string negative_file_name2;
} triplet_lfw_s;

//#define _LFW_MAKE_

int main(int argc, char** argv)
{
#ifdef USE_OPENCV
	//::google::InitGoogleLogging(argv[0]);
	// Print output to stderr (while still logging)
	FLAGS_alsologtostderr = 1;

#ifndef GFLAGS_GFLAGS_H_
	namespace gflags = google;
#endif

	gflags::SetUsageMessage("Convert a set of images to the leveldb/lmdb\n"
		"format used as input for Caffe.\n"
		"Usage:\n"
		"    convert_imageset [FLAGS] ROOTFOLDER/ LISTFILE DB_NAME\n"
		"The ImageNet dataset for the training demo is at\n"
		"    http://www.image-net.org/download-images\n");
	caffe::GlobalInit(&argc, &argv);

	if (argc < 4)
	{
		gflags::ShowUsageWithFlagsRestrict(argv[0], "tools/convert_triplet_data.cpp");
		return 1;
	}

	const bool is_color = !FLAGS_gray;
	const bool check_size = FLAGS_check_size;
	const bool encoded = FLAGS_encoded;
	const string encode_type = FLAGS_encode_type;

	std::ifstream infile(argv[2]);
	std::vector<triplet_s> lines_triplet;
	std::vector<triplet_lfw_s> lines_tri_lfw;

	std::string filename;
	std::string filename_pos, filename_neg, filename_neg2;
	int label;
	int label_pos, label_neg;
	int nReadCnt = 0;
		
#ifndef _LFW_MAKE_
	while (infile >> filename >> label >> filename_pos >> label_pos >> filename_neg >> label_neg)
	{
		triplet_s triplet_file;

		triplet_file.anchor_file_name = filename;
		triplet_file.positive_file_name = filename_pos;
		triplet_file.negative_file_name = filename_neg;
		triplet_file.anchor_label = label;
		triplet_file.positive_label = label_pos;
		triplet_file.negative_label = label_neg;

		lines_triplet.push_back(triplet_file);

		printf("%d of data is read.....\n", nReadCnt++);
	}
#else
	while (infile >> filename >> filename_pos >> filename_neg >> filename_neg2)
	{
		triplet_lfw_s tri_file;
		tri_file.anchor_file_name = filename;
		tri_file.positive_file_name = filename_pos;
		tri_file.negative_file_name = filename_neg;
		tri_file.negative_file_name2 = filename_neg2;

		lines_tri_lfw.push_back(tri_file);

		printf("%d of data is read.....\n", nReadCnt++);
	}
#endif

	if (FLAGS_shuffle)
	{
		// randomly shuffle data
		LOG(INFO) << "Shuffling data";
#ifndef _LFW_MAKE_ 
		shuffle(lines_triplet.begin(), lines_triplet.end());
#else
		shuffle(lines_tri_lfw.begin(), lines_tri_lfw.end());
#endif
	}
#ifndef _LFW_MAKE_
	LOG(INFO) << "A total of " << lines_triplet.size() << " images.";
#else
	LOG(INFO) << "A total of " << lines_tri_lfw.size() << " images.";
#endif

	if (encode_type.size() && !encoded)
	{
		LOG(INFO) << "encode_type specified, assuming encoded=true.";
	}

	int resize_height = std::max<int>(0, FLAGS_resize_height);
	int resize_width = std::max<int>(0, FLAGS_resize_width);

	// Create new DB
	scoped_ptr<db::DB> db(db::GetDB(FLAGS_backend));
	db->Open(argv[3], db::NEW);
	scoped_ptr<db::Transaction> txn(db->NewTransaction());

	// Storing to db
	std::string root_folder(argv[1]);
	Datum datum;
	Datum datum_pos;
	Datum datum_neg;
	Datum datum_neg2;
		
	int count = 0;
	const int kMaxKeyLength = 10;
	char key_cstr[kMaxKeyLength];
	int data_size = 0;

	bool data_size_initialized = false;

#ifndef _LFW_MAKE_
	for (int line_id = 0; line_id < lines_triplet.size(); ++line_id)
	{
		bool status, p_status, n_status;
		std::string enc = encode_type;
		if (encoded && !enc.size())
		{
			// Guess the encoding type from the file name
			string fn = lines_triplet[line_id].anchor_file_name;
			size_t p = fn.rfind('.');
			if (p == fn.npos)
			{
				LOG(WARNING) << "Failed to guess the encoding of '" << fn << "'";
			}
			enc = fn.substr(p);
			std::transform(enc.begin(), enc.end(), enc.begin(), ::tolower);
			
			string fnp = lines_triplet[line_id].positive_file_name;
			p = fnp.rfind('.');
			if (p == fnp.npos)
			{
				LOG(WARNING) << "Failed to guess the encoding of '" << fnp << "'";
			}
			enc = fnp.substr(p);
			std::transform(enc.begin(), enc.end(), enc.begin(), ::tolower); 
			
			string fnn = lines_triplet[line_id].negative_file_name;
			p = fnn.rfind('.');
			if (p == fnn.npos)
			{
				LOG(WARNING) << "Failed to guess the encoding of '" << fnn << "'";
			}
			enc = fnn.substr(p);
			std::transform(enc.begin(), enc.end(), enc.begin(), ::tolower);
		}
			
// 		status = ReadImageToDatum(root_folder + lines_triplet[line_id].anchor_file_name, root_folder + lines_triplet[line_id].positive_file_name,
// 			root_folder + lines_triplet[line_id].negative_file_name, lines_triplet[line_id].anchor_label, lines_triplet[line_id].positive_label,
// 			lines_triplet[line_id].negative_label, resize_height, resize_width, is_color, enc, &datum);

		status = ReadImageToDatum(root_folder + lines_triplet[line_id].anchor_file_name, lines_triplet[line_id].anchor_label,
			resize_height, resize_width, is_color, enc, &datum);
		p_status = ReadImageToDatum(root_folder + lines_triplet[line_id].positive_file_name, lines_triplet[line_id].positive_label, 
			resize_height, resize_width, is_color, enc, &datum_pos);
		n_status = ReadImageToDatum(root_folder + lines_triplet[line_id].negative_file_name, lines_triplet[line_id].negative_label, 
			resize_height, resize_width, is_color, enc, &datum_neg);

		printf("%d of data is added\n", line_id);
	
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (status == false || p_status == false || n_status == false)
		{
			continue;
		}

		if (check_size)
		{
			if (!data_size_initialized)
			{
				data_size = datum.channels() * datum.height() * datum.width();
				data_size_initialized = true;
			}
			else
			{
				const std::string &data = datum.data();
				const std::string &data_p = datum_pos.data();
				const std::string &data_n = datum_neg.data();

				CHECK_EQ(data.size(), data_size) << "Incorrect data field size " << data.size();
				CHECK_EQ(data_p.size(), data_size) << "Incorrect data field size " << data_p.size();
				CHECK_EQ(data_n.size(), data_size) << "Incorrect data field size " << data_n.size();
			}
		}

		// sequential		
		// Put in db
		string out, out_p, out_n;
		int length = sprintf_s(key_cstr, kMaxKeyLength, "%08d", count);

		CHECK(datum.SerializeToString(&out));
		txn->Put(string(key_cstr, length), out);
		count++;
		
		length = sprintf_s(key_cstr, kMaxKeyLength, "%08d", count);
		CHECK(datum_pos.SerializeToString(&out_p));
		txn->Put(string(key_cstr, length), out_p);
		count++;

		length = sprintf_s(key_cstr, kMaxKeyLength, "%08d", count);
		CHECK(datum_neg.SerializeToString(&out_n));
		txn->Put(string(key_cstr, length), out_n);
		count++;

		if (count % 1000 == 0)
		{
			// Commit db
			txn->Commit();
			txn.reset(db->NewTransaction());
			LOG(INFO) << "Processed " << count << " files.";
		}		
	}
#else
for (int line_id = 0; line_id < lines_tri_lfw.size(); ++line_id)
	{
		bool status, p_status, n_status, n_status2;
		std::string enc = encode_type;
		if (encoded && !enc.size())
		{
			// Guess the encoding type from the file name
			string fn = lines_tri_lfw[line_id].anchor_file_name;
			size_t p = fn.rfind('.');
			if (p == fn.npos)
			{
				LOG(WARNING) << "Failed to guess the encoding of '" << fn << "'";
			}
			enc = fn.substr(p);
			std::transform(enc.begin(), enc.end(), enc.begin(), ::tolower);

			string fnp = lines_tri_lfw[line_id].positive_file_name;
			p = fnp.rfind('.');
			if (p == fnp.npos)
			{
				LOG(WARNING) << "Failed to guess the encoding of '" << fnp << "'";
			}
			enc = fnp.substr(p);
			std::transform(enc.begin(), enc.end(), enc.begin(), ::tolower);

			string fnn = lines_tri_lfw[line_id].negative_file_name;
			p = fnn.rfind('.');
			if (p == fnn.npos)
			{
				LOG(WARNING) << "Failed to guess the encoding of '" << fnn << "'";
			}
			enc = fnn.substr(p);
			std::transform(enc.begin(), enc.end(), enc.begin(), ::tolower);

			string fnn2 = lines_tri_lfw[line_id].negative_file_name;
			p = fnn2.rfind('.');
			if (p == fnn.npos)
			{
				LOG(WARNING) << "Failed to guess the encoding of '" << fnn2 << "'";
			}
			enc = fnn2.substr(p);
			std::transform(enc.begin(), enc.end(), enc.begin(), ::tolower);
		}

		status = ReadImageToDatum(root_folder + lines_tri_lfw[line_id].anchor_file_name, resize_height, resize_width, is_color, enc, &datum);
		p_status = ReadImageToDatum(root_folder + lines_tri_lfw[line_id].positive_file_name, resize_height, resize_width, is_color, enc, &datum_pos);
		n_status = ReadImageToDatum(root_folder + lines_tri_lfw[line_id].negative_file_name, resize_height, resize_width, is_color, enc, &datum_neg);
		n_status2 = ReadImageToDatum(root_folder + lines_tri_lfw[line_id].negative_file_name2, resize_height, resize_width, is_color, enc, &datum_neg2);

		printf("%d of data is added\n", line_id);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (status == false || p_status == false || n_status == false)
		{
			continue;
		}

		if (check_size)
		{
			if (!data_size_initialized)
			{
				data_size = datum.channels() * datum.height() * datum.width();
				data_size_initialized = true;
			}
			else
			{
				const std::string &data = datum.data();
				const std::string &data_p = datum_pos.data();
				const std::string &data_n = datum_neg.data();
				const std::string &data_n2 = datum_neg2.data();

				CHECK_EQ(data.size(), data_size) << "Incorrect data field size " << data.size();
				CHECK_EQ(data_p.size(), data_size) << "Incorrect data field size " << data_p.size();
				CHECK_EQ(data_n.size(), data_size) << "Incorrect data field size " << data_n.size();
				CHECK_EQ(data_n2.size(), data_size) << "Incorrect data field size " << data_n2.size();
			}
		}

		// sequential		
		// Put in db
		string out, out_p, out_n, out_n2;
		int length = sprintf_s(key_cstr, kMaxKeyLength, "%08d", count);

		CHECK(datum.SerializeToString(&out));
		txn->Put(string(key_cstr, length), out);
		count++;

		length = sprintf_s(key_cstr, kMaxKeyLength, "%08d", count);
		CHECK(datum_pos.SerializeToString(&out_p));
		txn->Put(string(key_cstr, length), out_p);
		count++;

		length = sprintf_s(key_cstr, kMaxKeyLength, "%08d", count);
		CHECK(datum_neg.SerializeToString(&out_n));
		txn->Put(string(key_cstr, length), out_n);
		count++;

		length = sprintf_s(key_cstr, kMaxKeyLength, "%08d", count);
		CHECK(datum_neg2.SerializeToString(&out_n2));
		txn->Put(string(key_cstr, length), out_n2);
		count++;

		if (count % 1000 == 0)
		{
			// Commit db
			txn->Commit();
			txn.reset(db->NewTransaction());
			LOG(INFO) << "Processed " << count << " files.";
		}
	}

#endif
	// write the last batch
	if (count % 1000 != 0)
	{
		txn->Commit();
		LOG(INFO) << "Processed " << count << " files.";
	}
#else
	LOG(FATAL) << "This tool requires OpenCV; compile with USE_OPENCV.";
#endif  // USE_OPENCV
	return 0;
}
