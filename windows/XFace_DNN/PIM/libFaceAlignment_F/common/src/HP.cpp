#include "stdafx.h"

#include <time.h>
//#include <vector>
//#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include "HP_Defines.h"
#include "HP_Core.h"
#ifdef WIN32
#include <memory.h>
#endif
//using namespace std;

extern REGRESSOR *PoseR;
extern HP_CLASSIFIER *ROP_Classifier[3];

#if 0
const int Q_SIZE = 5;	
typedef struct _TRACKING_FACE_INFO
{
	HP_RECT rect;
	float yaw_q[Q_SIZE];
	float rip_q[Q_SIZE];
	int q_pos;
	clock_t last_matched_time;
} TRACKING_FACE_INFO;

vector<TRACKING_FACE_INFO *> tfi;
void HP_TimeAverage(HP_RECT &rect, float yaw, float rip, float *n_yaw,  float *n_rip)
{
	int track_idx = -1;
	for(int i = 0; i < tfi.size(); ++i)
	{
		HP_RECT &rc1 = rect;
		HP_RECT &rc2 = tfi[i]->rect;

		if((rc1.left-rc2.right)*(rc1.right-rc2.left) < 0 && (rc1.top-rc2.bottom)*(rc1.bottom-rc2.top) < 0)
		{
			if((min(rc1.right, rc2.right)-max(rc1.left, rc2.left))*(min(rc1.bottom, rc2.bottom)-max(rc1.top, rc2.top)) > 
				min((rc1.right-rc1.left)*(rc1.bottom-rc1.top), (rc2.right-rc2.left)*(rc2.bottom-rc2.top))/3)
			{
				track_idx = i;
				break;
			}
		}
	}
	if(track_idx == -1)
	{
		//새로운 얼굴 추가
		TRACKING_FACE_INFO *one_tfi = new TRACKING_FACE_INFO;
		one_tfi->q_pos = 0;
		memset(one_tfi->yaw_q, 0, sizeof(float)*Q_SIZE);
		memset(one_tfi->rip_q, 0, sizeof(float)*Q_SIZE);
		tfi.push_back(one_tfi);
		track_idx = tfi.size()-1;
	}

	int q_pos = ++tfi[track_idx]->q_pos;
	tfi[track_idx]->last_matched_time = clock();
	tfi[track_idx]->rect = rect;
	tfi[track_idx]->yaw_q[q_pos%Q_SIZE] = yaw;
	tfi[track_idx]->rip_q[q_pos%Q_SIZE] = rip;
	
	vector<float> yaw_v(Q_SIZE), rip_v(Q_SIZE);
	//memcpy(yaw_v.data(), tfi[track_idx]->yaw_q, Q_SIZE*sizeof(float));
	//memcpy(rip_v.data(), tfi[track_idx]->rip_q, Q_SIZE*sizeof(float));
	memcpy(&yaw_v.front(), tfi[track_idx]->yaw_q, Q_SIZE*sizeof(float));
	memcpy(&rip_v.front(), tfi[track_idx]->rip_q, Q_SIZE*sizeof(float));

	sort(yaw_v.begin(), yaw_v.end());
	sort(rip_v.begin(), rip_v.end());
	*n_yaw = 0; *n_rip = 0;
	for(int i = 1; i < Q_SIZE-1; ++i)
	{
		*n_yaw += yaw_v[i];
		*n_rip += rip_v[i];
	}
	*n_yaw /= (Q_SIZE-2);
	*n_rip /= (Q_SIZE-2);

	clock_t cur_clock = clock();
	while(1)
	{
		bool mismatch_found = false;
		for(vector<TRACKING_FACE_INFO *>::iterator it = tfi.begin(); it != tfi.end(); it++)
		{
			if(cur_clock-(*it)->last_matched_time > CLOCKS_PER_SEC) 
			{
				delete (*it);
				tfi.erase(it);
				mismatch_found = true;
				break;
			}
		}
		if(!mismatch_found) break;
	}
}

#else

#define Q_SIZE  5
#define MAX_TRACK_FACE 10*5 // MAX_FACE_SUPPORTED * 5

typedef struct TRACKING_FACE_INFO
{
	HP_RECT rect;
	float yaw_q[Q_SIZE];
	float roll_q[Q_SIZE];
	bool  valid[Q_SIZE];
	unsigned int q_pos;
	unsigned int last_matched_indx;
	char enable;
};


void HP_TimeAverage(HP_RECT rect, float yaw, float roll, float *n_yaw,  float *n_roll)
{
	//static vector<TRACKING_FACE_INFO *> tfi;
	static struct TRACKING_FACE_INFO tfi[MAX_TRACK_FACE] = {0};
	static unsigned char currindx = 0;

	int tfi_last_ind = -1;
	int track_idx = -1;
	
	for(int i = 0; i < MAX_TRACK_FACE; ++i)
	{
		if(((currindx - tfi[i].last_matched_indx) < MAX_TRACK_FACE)||(currindx < MAX_TRACK_FACE))
		{
			HP_RECT &rc1 = rect;
			HP_RECT &rc2 = tfi[i].rect;

			if((rc1.left-rc2.right)*(rc1.right-rc2.left) < 0 && (rc1.top-rc2.bottom)*(rc1.bottom-rc2.top) < 0)
			{
				if((HP_MIN(rc1.right, rc2.right)-HP_MAX(rc1.left, rc2.left))*(HP_MIN(rc1.bottom, rc2.bottom)-HP_MAX(rc1.top, rc2.top)) > 
					HP_MIN((rc1.right-rc1.left)*(rc1.bottom-rc1.top), (rc2.right-rc2.left)*(rc2.bottom-rc2.top))/3)
				{
					track_idx = i;
					break;
				}
			}
		}
		else if (tfi_last_ind == -1)
		{
			tfi_last_ind = i;
		}
	}


	if(track_idx == -1)
	{
		if(tfi_last_ind == -1) 
		{
			for(int i = 0; i < MAX_TRACK_FACE; i++)
			{
				if(!(tfi[++tfi_last_ind].enable))
				{
					break;
				}
			}
		}

		//새로운 얼굴 추가
		
		tfi[tfi_last_ind].q_pos = 0;
		memset(tfi[tfi_last_ind].yaw_q, 0 , sizeof(float)*Q_SIZE);
		memset(tfi[tfi_last_ind].roll_q, 0 , sizeof(float)*Q_SIZE);
		memset(tfi[tfi_last_ind].valid, 0 , sizeof(bool)*Q_SIZE);
		tfi[tfi_last_ind].enable = 1;
		track_idx = tfi_last_ind;


		//tfi_last_ind = (tfi_last_ind++)%MAX_TRACK_FACE;
	}

	unsigned int q_pos = ++tfi[track_idx].q_pos;
	tfi[track_idx].last_matched_indx = currindx;
	tfi[track_idx].rect = rect;
	tfi[track_idx].yaw_q[q_pos%Q_SIZE] = yaw;
	tfi[track_idx].roll_q[q_pos%Q_SIZE] = roll;
	tfi[track_idx].valid[q_pos%Q_SIZE] = true;
	
	float yaw_v[Q_SIZE];
	float roll_v[Q_SIZE];
	
	int valid_num = 0;
	for(int i = 0; i < Q_SIZE; ++i)
	{
		if(tfi[track_idx].valid[i]) 
		{
			yaw_v[valid_num] = tfi[track_idx].yaw_q[i];
			roll_v[valid_num] = tfi[track_idx].roll_q[i];
			valid_num++;
		}
	}

	{
		float temp;
		for(int i = 0; i < valid_num; i++)
		for(int j = i+1; j < valid_num; j++)
		{
			if(yaw_v[i] > yaw_v[j])
			{
				temp = yaw_v[i];
				yaw_v[i] = yaw_v[j];
				yaw_v[j] = temp;
			}

			if(roll_v[i] > roll_v[j])
			{
				temp = roll_v[i];
				roll_v[i] = roll_v[j];
				roll_v[j] = temp;
			}
		}
	}

	*n_yaw = 0; *n_roll = 0;
	if(valid_num > 2)
	{
		for(int i = 1; i < valid_num-1; ++i)
		{
			*n_yaw += yaw_v[i];
			*n_roll += roll_v[i];
		}
		*n_yaw /= (valid_num-2);
		*n_roll /= (valid_num-2);
	}
	else
	{
		*n_yaw = yaw;
		*n_roll = roll;
	}

	currindx++;
	/*
	clock_t cur_clock = clock();
	while(1)
	{
		bool mismatch_found = false;
		for(vector<TRACKING_FACE_INFO *>::iterator it = tfi.begin(); it != tfi.end(); it++)
		{
			if(cur_clock-(*it)->last_matched_time > CLOCKS_PER_SEC) 
			{
				delete (*it);
				tfi.erase(it);
				mismatch_found = true;
				break;
			}
		}
		if(!mismatch_found) break;
	}
	*/
}

#endif

void normalize_pts(int pt_cnt, HP_POINTF pt[], HP_POINTF center, double rip_angle, double dx, double dy, double scale, HP_POINTF dst_pt[])
{
	double rad_angle = 3.141592/180.0*rip_angle;
	HP_POINTF temp_pt[100];
	for(int i = 0; i < pt_cnt; i++)
	{
		temp_pt[i].x = (pt[i].x-center.x)*scale+0.5;
		temp_pt[i].y = (pt[i].y-center.y)*scale+0.5;
	}
	for(int i = 0; i < pt_cnt; i++)
	{
		dst_pt[i].x = temp_pt[i].x*cos(rad_angle)+temp_pt[i].y*sin(rad_angle);
		dst_pt[i].y = -(temp_pt[i].x*sin(rad_angle)-temp_pt[i].y*cos(rad_angle));

		dst_pt[i].x += center.x + dx;
		dst_pt[i].y += center.y + dy;
	}
}
bool InvMatrix3x3(float a[3][3], float a_i[3][3])
{
	float &a11 = a[0][0], &a12 = a[0][1], &a13 = a[0][2];
	float &a21 = a[1][0], &a22 = a[1][1], &a23 = a[1][2];
	float &a31 = a[2][0], &a32 = a[2][1], &a33 = a[2][2];
	float det = a11*a22*a33 + a21*a32*a13 + a31*a12*a23 - a11*a32*a23 - a31*a22*a13 - a21*a12*a33;
	if(det == 0) return false;
	float det_inv = 1.0f/det;
	a_i[0][0] = det_inv*(a22*a33-a23*a32); a_i[0][1] = det_inv*(a13*a32-a12*a33); a_i[0][2] = det_inv*(a12*a23-a13*a22);
	a_i[1][0] = det_inv*(a23*a31-a21*a33); a_i[1][1] = det_inv*(a11*a33-a13*a31); a_i[1][2] = det_inv*(a13*a21-a11*a23);
	a_i[2][0] = det_inv*(a21*a32-a22*a31); a_i[2][1] = det_inv*(a12*a31-a11*a32); a_i[2][2] = det_inv*(a11*a22-a12*a21);
	return true;
}
void MatrixMul3x3(float a[3][3], float b[3][3], float c[3][3])
{
	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			float sum = 0;
			for(int k = 0; k < 3; ++k)
			{
				sum += a[i][k] * b[k][j];
			}
			c[i][j] = sum;
		}
	}
}
void affine_pts(HP_POINTF source_pt[3], HP_POINTF target_pt[3], int pt_cnt, HP_POINTF *pt_in, HP_POINTF *pt_out)
{
	float target[3][3] = {target_pt[0].x, target_pt[1].x, target_pt[2].x,
						  target_pt[0].y, target_pt[1].y, target_pt[2].y,
						  1			    , 1				, 1};
	float source[3][3] = {source_pt[0].x, source_pt[1].x, source_pt[2].x,
						  source_pt[0].y, source_pt[1].y, source_pt[2].y,
						  1			    , 1				, 1};
	float param[3][3], i_source[3][3];
	InvMatrix3x3(source, i_source);
	MatrixMul3x3(target, i_source, param);
	for(int i = 0; i < pt_cnt; i++)
	{
		pt_out[i].x = pt_in[i].x*param[0][0]+pt_in[i].y*param[0][1]+param[0][2];
		pt_out[i].y = pt_in[i].x*param[1][0]+pt_in[i].y*param[1][1]+param[1][2];
	}
}

float Interpol(float samples[], int samples_num, float input_range_min, float input_range_max, float input)
{
	float pos = (input-input_range_min)/(input_range_max-input_range_min);
	float spos = pos*(samples_num-1);
	int idx = HP_CLIP(int(spos), 0, samples_num-1);
	int next_idx = HP_CLIP(int(spos+1), 0, samples_num-1);
	float residue = fabs(spos)-abs(int(spos));
	float v = samples[idx]*(1.0f-residue) + samples[next_idx]*residue;
	return v;
}


void ComputeWarpParams(HP_POINTF src_pts[3], HP_POINTF target_pts[3], float param[2][3])
{
	float M1[3][3] = {src_pts[0].x, src_pts[1].x, src_pts[2].x,
					  src_pts[0].y, src_pts[1].y, src_pts[2].y,
					  1			  , 1			, 1};

	float M2[3][3] = {target_pts[0].x, target_pts[1].x, target_pts[2].x,
					  target_pts[0].y, target_pts[1].y, target_pts[2].y,
					  1				 , 1			  , 1};
	float M2_i[3][3], result[3][3];
	InvMatrix3x3(M2, M2_i);
	MatrixMul3x3(M1, M2_i, result);

	param[0][0] = result[0][0]; param[0][1] = result[0][1]; param[0][2] = result[0][2];
	param[1][0] = result[1][0]; param[1][1] = result[1][1]; param[1][2] = result[1][2];
}
//#include "./vcprj/ImageProcessing.h"
void HP_EstimatePose(unsigned char *rawdata, int src_w, int src_h, HP_RECT face_rect, int fd_rip, int fd_rop, int image_direction, float *rip_angle, float *rop_angle, HP_POINTF *estimated_shape)
{
	HP_POINTF pt[4], fd_pts[4], center;
	pt[0].x = face_rect.left;
	pt[0].y = face_rect.top;
	pt[1].x = face_rect.right;
	pt[1].y = face_rect.top;
	pt[2].x = face_rect.right;
	pt[2].y = face_rect.bottom;
	pt[3].x = face_rect.left;
	pt[3].y = face_rect.bottom;
	center.x = (pt[0].x+pt[1].x)/2;
	center.y = (pt[1].y+pt[2].y)/2;
	normalize_pts(4, pt, center, fd_rip, 0, 0, 1.0, fd_pts);
	
	HP_POINTF pose_face_pts[4];
	HP_POINTF normalfd_pts[4];
	HP_POINTF normalfd_pts_top_up[4] = {{30, 30}, {60, 30}, {60, 60}, {30, 60}};
	HP_POINTF normalfd_pts_left_up[4] = {{60, 30}, {60, 60}, {30, 60}, {30, 30}};
	HP_POINTF normalfd_pts_right_up[4] = {{30, 60}, {30, 30}, {60, 30}, {60, 60}};

	float base_rip = 0;
	if(image_direction == 0) memcpy(normalfd_pts, normalfd_pts_top_up, sizeof(HP_POINTF)*4);
	if(image_direction == 1) 
	{
		base_rip = 90;
		memcpy(normalfd_pts, normalfd_pts_left_up, sizeof(HP_POINTF)*4);
	}
	if(image_direction == 2) 
	{
		base_rip = -90;
		memcpy(normalfd_pts, normalfd_pts_right_up, sizeof(HP_POINTF)*4);
	}
	
	unsigned char patch[IMAGE_WIDTH*IMAGE_HEIGHT];
	unsigned char rop_patch[40*40];
	float warp_param[2][3];

	ComputeWarpParams(fd_pts, normalfd_pts, warp_param);
	WarpImage(rawdata, src_w, src_h, patch, IMAGE_WIDTH, IMAGE_HEIGHT, warp_param);
	
	/*
	CImageProcessing ip;
	CImage patchimg;
	ip.setraw(patchimg, IMAGE_WIDTH, IMAGE_HEIGHT, 1, patch);
	for(int i = 0, j = 3; i < 4; j=i, i++)
	{
		ip.draw_line(patchimg, normalfd_pts[j].x, normalfd_pts[j].y, normalfd_pts[i].x, normalfd_pts[i].y, 255, 1);
	}
	cvNamedWindow("t2");
	cvShowImage("t2", patchimg.GetImage());
	*/

	float *var = PoseR->var;
	float *mean = PoseR->mean;

	int rop_idx;
	if(HP_BETWEEN(fd_rop, -90, -30)) rop_idx = 0;
	else if(HP_BETWEEN(fd_rop, -30, +30)) rop_idx = 1;
	else if(HP_BETWEEN(fd_rop, +30, +90)) rop_idx = 2;
	
	const int try_num = 5;
	float p_try[try_num][5], p[5];
	for(int i = 0; i < try_num; ++i)
	{
/*		float p_i[4] = {((45.0+rand()/float(RAND_MAX)*3.0-1.5)-mean[0])/var[0], 
						((45.0+rand()/float(RAND_MAX)*3.0-1.5)-mean[1])/var[1],
						((30.0+rand()/float(RAND_MAX)*5.0-2.5)-mean[2])/var[2],
						(0.0-mean[3])/var[3]};*/
		float p_i[4] = {((45.0)-mean[0])/var[0], 
						((45.0)-mean[1])/var[1],
						((30.0)-mean[2])/var[2],
						(0.0-mean[3])/var[3]};	
		float eval_p[5];
		EstimatePoseRect(patch, rop_idx,  p_i, eval_p, PoseR);
		
		HP_POINTF affine_normal_gt_pts[4];
		float cx = eval_p[0]*var[0]+mean[0];
		float cy = eval_p[1]*var[1]+mean[1];
		float size = eval_p[2]*var[2]+mean[2];
		float rip = eval_p[3]*var[3]+mean[3];
		HP_POINTF normal_gt_pts[] = {{-size/2.0f, -size/2.0f}, {+size/2.0f, -size/2.0f}, {+size/2.0f, +size/2.0f}, {-size/2.0f, +size/2.0f}};
		center.x = 0; center.y = 0;
		normalize_pts(4, normal_gt_pts, center, rip, cx, cy, 1.0f, affine_normal_gt_pts);
		affine_pts(normalfd_pts, fd_pts, 4, affine_normal_gt_pts, pose_face_pts);
		HP_POINTF normal_rop_pts[4] = {{0, 0}, {40, 0}, {40, 40}, {0, 40}};
		ComputeWarpParams(pose_face_pts, normal_rop_pts, warp_param);
		WarpImage(rawdata, src_w, src_h, rop_patch, 40, 40, warp_param);
		/*
		{
			CImageProcessing ip;
			CImage patchimg;
			ip.setraw(patchimg, 40, 40, 1, rop_patch);
			cvNamedWindow("t2");
			cvShowImage("t2", patchimg.GetImage());
		}
		*/
		eval_p[4] = ComputeROP(rop_patch, 40, 40, ROP_Classifier[rop_idx], rop_idx);
		
		for(int j = 0; j < 5; ++j) p_try[j][i] = eval_p[j];
	}
	
	for(int i = 0; i < 5; ++i) 
	{
		float elem[try_num];
		memcpy(elem, p_try[i], sizeof(float)*try_num);
		for(int j = 0; j < 3; ++j)
		{
			float mean = 0;			
			for(int k = 0; k < try_num; ++k) 
			{
				if(elem[k] != - 99999) mean += elem[k];
			}
			mean /= (try_num-j);
			if(j == 2) 
			{
				p[i] = mean;
				break;
			}
			int max_diff_idx = -1;
			for(int k = 0; k < try_num; ++k)
			{
				if(elem[k] != -99999)
				{
					if(max_diff_idx == -1) max_diff_idx = k;
					else if(abs(elem[k]-mean) > abs(elem[max_diff_idx]-mean)) max_diff_idx = k;
				}
			}
			elem[max_diff_idx] = -99999;
		}
	}

	HP_POINTF affine_normal_gt_pts[4];
	
	float cx = p[0]*var[0]+mean[0];
	float cy = p[1]*var[1]+mean[1];
	float size = p[2]*var[2]+mean[2];
	float rip = p[3]*var[3]+mean[3];
	
	float rop_angles[9] = {-90, -60, -34, -16, 0, 16, 34, 60, 90};
	float rop = Interpol(rop_angles, 9, -4, +4, p[4]);
	
	/*
	HP_POINTF normal_gt_pts[] = {{-size/2.0f, -size/2.0f}, {+size/2.0f, -size/2.0f}, {+size/2.0f, +size/2.0f}, {-size/2.0f, +size/2.0f}};

	center.x = 0; center.y = 0;
	normalize_pts(4, normal_gt_pts, center, rip, cx, cy, 1.0f, affine_normal_gt_pts);
	
	
	affine_pts(normalfd_pts, fd_pts, 4, affine_normal_gt_pts, pose_face_pts);
	if(estimated_shape) memcpy(estimated_shape, pose_face_pts, sizeof(HP_POINTF)*4);
	float dist = sqrtf(powf(pose_face_pts[1].x-pose_face_pts[0].x, 2.0)+powf(pose_face_pts[1].y-pose_face_pts[0].y, 2.0));
	*rip_angle = asin((pose_face_pts[0].y-pose_face_pts[1].y)/dist)*180/3.141592;
	*rop_angle = rop;
	*/

	switch(image_direction)
	{
	case 0:
		*rip_angle = fd_rip+rip;
		break;
	case 1:
		*rip_angle = fd_rip+rip+90;
		break;
	case 2:
		*rip_angle = fd_rip+rip-90;
		break;
	case 3:
		if(fd_rip >= 0) *rip_angle = 180-abs(fd_rip);
		else *rip_angle = -180+abs(fd_rip);
	}
	
	*rop_angle = rop;
}

