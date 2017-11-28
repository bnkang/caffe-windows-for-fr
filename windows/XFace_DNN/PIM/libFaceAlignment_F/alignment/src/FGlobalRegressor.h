#ifndef _GLOBAL_FREGRESSOR_H_
#define _GLOBAL_FREGRESSOR_H_

#include "FMat.h"
#include "IMat.h"

class CFGlobalRegressor{
public:
	CFGlobalRegressor();
	~CFGlobalRegressor();

	void set_num_stage(int t);
	void set_weight(CFMat &w, int stage);
	void size(int stage, int &h, int &w);
	CFMat *get_weight(int stage);
	CFMat update_residual(int stage, CIMat &local_binary_features);

private:
	int T;
	CFMat *W;
	
};

#endif