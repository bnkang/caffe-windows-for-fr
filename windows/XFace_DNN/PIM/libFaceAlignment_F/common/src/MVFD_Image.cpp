#include "stdafx.h"

#include "MVFD_ImageProcessing.h"
#include "MVFD_Defines.h"
#include "MVFD_Interface.h"
#include "MVFD_Global.h"
#include "MVFD_Enroll.h"
#include "MVFD_Core.h"
#include "MVFD_Memory.h"
int MVFD_DetectFace_ROI(FD_CONTEXT *pContext, unsigned char *src, int src_w, int src_h, int min_face_sz, int max_face_sz, int img_orientation, FD_INTERNAL_RESULT result[]);
void MVFD_Run_Detector_Threads(FD_CONTEXT *pContext, unsigned char *image, int width, int height, int img_orientation, int min_face_sz, int max_face_sz, float scale_rate);
 void *MVFD_Detector_Thread(void *pArg);

//Detect faces in the image
int MVFD_DetectFace_Image(FD_CONTEXT *pContext, unsigned char *src, int src_w, int src_h, FD_RECT roi, int min_face_sz, int max_face_sz, int img_orientation)
{
	//printf("roi left - %d, top - %d, right - %d, bottom - %d, src_w - %d, src_h - %d\n", roi.left, roi.top, roi.right, roi.bottom, src_w, src_h);
	if(roi.left == 0 && roi.top == 0 && roi.right == src_w && roi.bottom == src_h)
	{
		//printf("call 1\n");
		pContext->m_nFinalResultCnt = MVFD_DetectFace_ROI(pContext, src, src_w, src_h, min_face_sz, max_face_sz, img_orientation, pContext->m_FinalResults);
		return pContext->m_nFinalResultCnt;
	}
	else
	{
		//printf("call 2\n");
		unsigned char *roi_buf = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_REGION_BUF, (roi.right-roi.left)	*(roi.bottom-roi.top));	
		MVFD_CropRegion(src, src_w, src_h, roi, roi_buf);
	
		pContext->m_nFinalResultCnt = MVFD_DetectFace_ROI(pContext, roi_buf, roi.right-roi.left, roi.bottom-roi.top, min_face_sz, max_face_sz, img_orientation, pContext->m_FinalResults);
		
		for(int i = 0; i < pContext->m_nFinalResultCnt; ++i)
		{
			pContext->m_FinalResults[i].rect.left += roi.left;
			pContext->m_FinalResults[i].rect.top  += roi.top;
			pContext->m_FinalResults[i].rect.right += roi.left;
			pContext->m_FinalResults[i].rect.bottom += roi.top;
		}
		return pContext->m_nFinalResultCnt;
	}
	return 0;
}

//#include "ImageProcessing.h"
int MVFD_DetectFace_ROI(FD_CONTEXT *pContext, unsigned char *src, int src_w, int src_h, int min_face_sz, int max_face_sz, int img_orientation, FD_INTERNAL_RESULT result[])
{
	MVFD_ClearFaceEnrollStorage(&pContext->detect_enroll_faces);
	
	if(img_orientation == ROTATE_SDIC_SEARCH) img_orientation = ROTATE_180_SEARCH;

	int cur_orientation = (img_orientation == ROTATE_180_SEARCH || img_orientation == ROTATE_FULL_SEARCH) ? 0 : img_orientation;
	
	while(cur_orientation < 4)
	{
		unsigned char *rot_src;
		int rot_src_w, rot_src_h;
		if(cur_orientation == ROTATE_TOP_UP) rot_src = src;
		else 
		{
			rot_src = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_ROTATION_IMG, src_w*src_h);
			MVFD_MT_Rotate_RIP(src, src_w, src_h, rot_src, cur_orientation, 0, src_h);
		}
		if(cur_orientation == ROTATE_TOP_UP || cur_orientation == ROTATE_BOTTOM_UP)
		{
			rot_src_w = src_w;
			rot_src_h = src_h;
		}
		if(cur_orientation == ROTATE_LEFT_UP || cur_orientation == ROTATE_RIGHT_UP)
		{
			rot_src_w = src_h;
			rot_src_h = src_w;
		}

		MVFD_Run_Detector_Threads(pContext, rot_src, rot_src_w, rot_src_h, cur_orientation, min_face_sz, max_face_sz, FD_IMAGE_SCALE_RATE);

		if(img_orientation != ROTATE_180_SEARCH && img_orientation != ROTATE_FULL_SEARCH) break;
		if(img_orientation == ROTATE_180_SEARCH && cur_orientation == 2) break;
		cur_orientation++;
	}

	return MVFD_BoxClustering(&pContext->detect_enroll_faces, FD_MIN_INTERSECTION_CNT, result);
}

#ifndef WIN32
#include <unistd.h>
void FD_Sleep(int ms)
{
	sleep(ms);
}
#else
#include <Windows.h>
void FD_Sleep(int ms)
{
	Sleep(ms);
}
#endif
void MVFD_Run_Detector_Threads(FD_CONTEXT *pContext, unsigned char *image, int width, int height, int img_orientation, int min_face_sz, int max_face_sz, float scale_rate)
{
	if(min_face_sz == DEFAULT_SIZE) min_face_sz = FD_MAX(FD_MIN(width/20, height/20), FD_FACE_WIDTH);
	if(max_face_sz == DEFAULT_SIZE) max_face_sz = FD_MIN(width, height);

	float init_scale = FD_FACE_WIDTH/float(min_face_sz);
	float last_scale = FD_FACE_WIDTH/float(max_face_sz);
	float scale = init_scale;

	int resize_w = width*init_scale;
	int resize_h = height*init_scale;
	int margin_resize_w = resize_w + FD_FACE_WIDTH;
	int margin_resize_h = resize_h + FD_FACE_HEIGHT;
	unsigned char *resize_buf = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_RESIZE_BUF, margin_resize_w*(margin_resize_h+FD_FACE_HEIGHT));	
	unsigned char *bilinear_map = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_BILINEAR_MAP, (width<<2)*height);
	MVFD_Make_ResizeBI_Map(image, width, height, bilinear_map);
	while(scale > last_scale)
	{
		#ifdef ENABLE_CLIPPED_FACE_DETECTION
		if(resize_w < 100 || resize_h < 100)
		{
			margin_resize_w = resize_w+FD_FACE_WIDTH;
			margin_resize_h = resize_h+FD_FACE_HEIGHT;	
		}
		else
		#endif
		{
			margin_resize_w = resize_w;
			margin_resize_h = resize_h;
		}

		if(margin_resize_w < 36 || margin_resize_h < 36) break;	

		FD_RECT des_roi = {(margin_resize_w-resize_w)/2, (margin_resize_h-resize_h)/2, 
							(margin_resize_w-resize_w)/2+resize_w, (margin_resize_h-resize_h)/2+resize_h};
		FD_RECT bimap_roi = {0, 0, width, height};
		if(!MVFD_MT_ResizeBI_R2R_UseMap(bilinear_map, width, height, bimap_roi, resize_buf, margin_resize_w, margin_resize_h, des_roi)) continue;

		int active_thread_num = FD_MAX_THREADS_NUM;
		int sub_div_height;
		while(1)
		{
			sub_div_height = (margin_resize_h-FD_FACE_HEIGHT+active_thread_num-1)/active_thread_num;
			sub_div_height += (sub_div_height&1);		//make it even number
			if(sub_div_height > 20 || active_thread_num == 1) break;
			active_thread_num--;
		}

		pContext->subImage_width = margin_resize_w;
		pContext->subImage_height = sub_div_height+FD_FACE_HEIGHT-1;
		
		for(int j  = 1; j <= FD_MAX_CLASS_NUM; ++j)
		{
			for(int k = 0; k < FD_MAX_STAGE_NUM; ++k)
			{
				if(MVFD_Detector[j][k]) MVFD_RescaleFeaturePositions(pContext, MVFD_Detector[j][k], pContext->subImage_width, pContext->subImage_height, pContext->det_ffpos.face[j][k]);
			}
		}

		MVFD_RescaleFeaturePositions(pContext, MVFD_PoseClassifier, pContext->subImage_width, pContext->subImage_height, pContext->det_ffpos.pose_map);
				
		pContext->finish_count = 0;
		pContext->worker_num = active_thread_num;
		for(int j = 0; j < active_thread_num; ++j)
		{
			pContext->subImage[j] = resize_buf+j*pContext->subImage_width*sub_div_height;
			MVFD_Mutex_UnLock(&pContext->wakeup_mutex[j]);
		}
		MVFD_Mutex_Lock(&pContext->finish_wait_mutex);

		for(int j = 0; j < active_thread_num; ++j)
		{
			int (*sub_xy_buf)[4] = (int (*)[4])pContext->face_candi_xy_buf[j];
			int face_cnt = pContext->found_face_cnt[j];
			for(int k = 0; k < face_cnt; ++k)
			{
				MVFD_EnrollFace2(sub_xy_buf[k][0]-des_roi.left, (j*sub_div_height+sub_xy_buf[k][1])-des_roi.top, width/float(resize_w), resize_w, resize_h, 0, 0,
							sub_xy_buf[k][2], img_orientation, sub_xy_buf[k][3], &pContext->detect_enroll_faces);
			}
		}

		scale *= scale_rate;
		resize_w = width*scale+0.5f;
		resize_h = height*scale+0.5f;
	}
}

 void *MVFD_Detector_Thread(void *pArg)
{
	FD_THREAD_PARAM *pParam = (FD_THREAD_PARAM *)pArg;
	int thread_idx = pParam->thread_idx;
	FD_CONTEXT *pContext = (FD_CONTEXT *)pParam->pArg;
	
#ifdef USE_NEON
	unsigned int *scratchBuffer = MVFD_MemSection_Alloc<unsigned int>(pContext, MEM_SECTION_ISFACE_SCRATCH_BUFFER, 300*6);//Max feature num is < 300		
	#endif
	while(1)
	{
		MVFD_Mutex_Lock(&pContext->wakeup_mutex[thread_idx]);
		if(pContext->wait_for_exit) break;
		int face_cnt = 0;
		int width = pContext->subImage_width;
		int height = pContext->subImage_height;
		int *face_xy = pContext->face_candi_xy_buf[thread_idx];
		unsigned char *pimage = pContext->subImage[thread_idx];
		
		int feature_tot_buf_sz = width*height*3;
		feature_tot_buf_sz = ALIGN16(feature_tot_buf_sz) +  FD_MAX(IRMB2_ALIGNED_WIDTH(width)*height*4, IRMB2_ALIGNED_WIDTH(height)*width*4);
		unsigned char *feature_buf = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_FEATURE_BUF, feature_tot_buf_sz);
		int feature_buf_sz = 0;
		FEATURE feature;
		feature.img = (unsigned char *)pimage;
		feature.mct = (unsigned short *)feature_buf;
		feature_buf_sz += width*height*sizeof(unsigned short);
		feature.rot = (unsigned char *)&feature_buf[feature_buf_sz];
		feature_buf_sz += width*height;
		feature.irmb = (unsigned char *)&feature_buf[ALIGN16(feature_buf_sz)];
		feature.width = width;
		feature.height = height;
		MVFD_MT_Image2MCT(pimage, width, height, feature.mct, 0, feature.height);

		MVFD_Image2IRMB(pimage, width, height, pContext, feature.irmb);
		
		#ifndef USE_NEON
		MVFD_ImageRotate(pimage, feature.width, feature.height, feature.rot);
		#else
		MVFD_ImageTranspose(pimage, feature.width, feature.height, feature.rot);
		#endif
		unsigned short *pose_map = MVFD_MemSection_Alloc<unsigned short>(pContext, MEM_SECTION_POSE_MAP, width*height);
		#ifndef USE_NEON
		MVFD_GeneratePoseMap(pContext, MVFD_BkgndRejector, MVFD_PoseClassifier, &feature, FD_REJECTION_RATE, pose_map, pContext->det_ffpos.pose_map);
		#else
		MVFD_GeneratePoseMap(pContext, MVFD_BkgndRejector, MVFD_PoseClassifier, &feature, pose_map, pContext->det_ffpos.pose_map);
		#endif

		for(int y = 0; y < height-FD_FACE_HEIGHT; y +=2)
		{
			for(int x = 0; x < width-FD_FACE_WIDTH; x +=2)
			{
				#ifdef FD_NO_REJECTOR_TO_FRONTAL_FACE
				int confidence;
				#ifdef USE_NEON
				confidence = MVFD_IsFace(&feature, x, y, MVFD_Detector[9][0], scratchBuffer);
				#else
				confidence = MVFD_IsFace(&feature, x, y, MVFD_Detector[9][0], pContext->det_ffpos.face[9]);
				#endif
				if(confidence)
				{
					if(face_cnt < FD_MAX_FACE_CANDIDATES_PER_THREAD)
					{
						*face_xy++ = x; *face_xy++ = y; *face_xy++ = 9; *face_xy++ = confidence;
						face_cnt++;
					}
					continue;
				}
				#endif
				int pose = pose_map[y*feature.width+x];
				if(pose != 0xffff)
				{
					#ifndef FD_NO_REJECTOR_TO_FRONTAL_FACE
					//frontal face detector is directly applied, regardless of result of the pose classifier.
					int confidence;
					#ifdef USE_NEON
					confidence  = MVFD_IsFace(&feature, x, y, MVFD_Detector[9][0], scratchBuffer, pContext->det_ffpos.face[9]);
					#else
					confidence  = MVFD_IsFace(&feature, x, y, MVFD_Detector[9][0], pContext->det_ffpos.face[9]);
					#endif
					if(confidence)
					{
						if(face_cnt < FD_MAX_FACE_CANDIDATES_PER_THREAD)
						{
							*face_xy++ = x; *face_xy++ = y; *face_xy++ = 9; *face_xy++ = confidence;
							face_cnt++;
						}
						continue;
					}
					#endif
					//if it belong to one of 8 poses.
					if(pose != 0)
					{
						int confidence;
						int pose1 = pose & 0xff;	// pose of maximum confidence
						int pose2 = (pose & 0xff00) >> 8;  // pose of second confidence
						if(MVFD_Detector[pose1][0])
						{
							#ifdef USE_NEON
							confidence = MVFD_IsFace(&feature, x, y, MVFD_Detector[pose1][0], scratchBuffer, pContext->det_ffpos.face[pose1]);
							#else
							confidence = MVFD_IsFace(&feature, x, y, MVFD_Detector[pose1][0], pContext->det_ffpos.face[pose1]);
							#endif
							if(confidence)
							{
								if(face_cnt < FD_MAX_FACE_CANDIDATES_PER_THREAD)
								{
									*face_xy++ = x; *face_xy++ = y; *face_xy++ = pose1; *face_xy++ = confidence;
									face_cnt++;
								}
								continue;
							}
						}
					
						if(MVFD_Detector[pose2][0])
						{
							#ifdef USE_NEON
							confidence = MVFD_IsFace(&feature, x, y, MVFD_Detector[pose2][0], scratchBuffer, pContext->det_ffpos.face[pose2]);
							#else
							confidence = MVFD_IsFace(&feature, x, y, MVFD_Detector[pose2][0], pContext->det_ffpos.face[pose2]);
							#endif
							if(confidence)
							{
								if(face_cnt < FD_MAX_FACE_CANDIDATES_PER_THREAD)
								{
									*face_xy++ = x; *face_xy++ = y; *face_xy++ = pose2; *face_xy++ = confidence;
									face_cnt++;
								}
							}
						}
					}				
				}

			}
		}

		pContext->found_face_cnt[thread_idx] = face_cnt;
		
		MVFD_Mutex_Lock(&pContext->finish_count_mutex);
		pContext->finish_count++;
		if(pContext->finish_count == pContext->worker_num)
		{
			MVFD_Mutex_UnLock(&pContext->finish_wait_mutex);
		}
		MVFD_Mutex_UnLock(&pContext->finish_count_mutex);

	}
	delete pParam;
	return NULL;
}
