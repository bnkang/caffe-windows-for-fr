#ifndef _FACE_REJECTOR_H_
#define _FACE_REJECTOR_H_

#include "LBFCommon.h"

class CFaceRejector {
public:
	CFaceRejector();
	~CFaceRejector();

public:
	bool is_face(data_info &data);

private:
	bool normalize(data_info &data, unsigned char *des_data);
	bool classify(unsigned char* nSrcData);

private:
	int	*nLBPFeatureGlobalPosition;
};

#endif