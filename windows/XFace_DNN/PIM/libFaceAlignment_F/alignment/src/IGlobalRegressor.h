#ifndef _GLOBAL_IREGRESSOR_H_
#define _GLOBAL_IREGRESSOR_H_

#include "FMat.h"
#include "IMat.h"

class CIGlobalRegressor{
public:
	CIGlobalRegressor();
	~CIGlobalRegressor();

	void set_num_stage(int t);
	void set_weight(CIMat &w, int stage);
	void size(int stage, int &h, int &w);
	CIMat *get_weight(int stage);
	CFMat update_residual(int stage, CIMat &local_binary_features);

private:
	int T;
	CIMat *W;
	CIMat *Wt;
	
};

#endif