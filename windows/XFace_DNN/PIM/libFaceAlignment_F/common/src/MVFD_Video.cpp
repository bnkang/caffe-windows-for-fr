#include "stdafx.h"

#include "MVFD_Global.h"
#include "MVFD_Memory.h"
#include "MVFD_Enroll.h"
#include "MVFD_Core.h"
#include "MVFD_ImageProcessing.h"
#include "MVFD_Thread.h"
void MVFD_ComputeVideoScale(FD_CONTEXT *pContext, int src_w, int src_h, float init_scale_rate, float last_scale_rate);
int MVFD_VideoTrackingFaces(FD_CONTEXT *pContext, unsigned char *src, int src_w, int src_h, FD_INTERNAL_RESULT *prev_results, int prev_results_num, int rotation, bool half_margin, FD_INTERNAL_RESULT *track_results);
int MVFD_VideoDetectFaces(FD_CONTEXT *pContext, unsigned char *src, int src_w, int src_h, int img_orientation, FD_INTERNAL_RESULT *detect_results);
int MVFD_RemoveOverlappedFaces(FD_INTERNAL_RESULT src_faces[], int src_num, FD_INTERNAL_RESULT remove_faces[], int remove_num);
int MVFD_MergeFaces_UseConfidence(FD_INTERNAL_RESULT src_faces1[], int src_num1, FD_INTERNAL_RESULT src_faces2[], int src_num2,  FD_INTERNAL_RESULT new_faces[]);
int MVFD_DetectFace_Video_subImage(FD_CONTEXT *pContext, unsigned char *src, int src_w, int src_h, int min_face_sz, int max_face_sz, int rotation);
void MVFD_Run_Detector_Threads(FD_CONTEXT *pContext, unsigned char *image, int width, int height, int img_orientation, int min_face_sz, int max_face_sz, float scale_rate);

typedef struct
{
	FD_RECT roi;
	int resize_w;
	int resize_h;
	FEATURE feature;
} IMG_PATCH_INFO;

void showimg(unsigned char *image, int width, int height);

int MVFD_DetectFace_Video(FD_CONTEXT *pContext, unsigned char *src, int src_w, int src_h, FD_RECT roi, int min_face_sz, int max_face_sz, int rotation)
{
	if(roi.left == 0 && roi.top == 0 && roi.right == src_w && roi.bottom == src_h)
	{
		MVFD_DetectFace_Video_subImage(pContext, src, src_w, src_h, min_face_sz, max_face_sz, rotation);
	}
	else
	{
		unsigned char *roi_buf = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_REGION_BUF, (roi.right-roi.left)*(roi.bottom-roi.top));	
		MVFD_CropRegion(src, src_w, src_h, roi, roi_buf);
	
		for(int i = 0; i < pContext->m_nFinalResultCnt; ++i)
		{
			pContext->m_FinalResults[i].rect.left -= roi.left;
			pContext->m_FinalResults[i].rect.top  -= roi.top;
			pContext->m_FinalResults[i].rect.right -= roi.left;
			pContext->m_FinalResults[i].rect.bottom -= roi.top;
		}
		MVFD_DetectFace_Video_subImage(pContext, roi_buf, roi.right-roi.left, roi.bottom-roi.top, min_face_sz, max_face_sz, rotation);
	
		for(int i = 0; i < pContext->m_nFinalResultCnt; ++i)
		{
			pContext->m_FinalResults[i].rect.left += roi.left;
			pContext->m_FinalResults[i].rect.top  += roi.top;
			pContext->m_FinalResults[i].rect.right += roi.left;
			pContext->m_FinalResults[i].rect.bottom += roi.top;
		}
	}
	//print_memory_details();
	return pContext->m_nFinalResultCnt;
}
int MVFD_DetectFace_Video_subImage(FD_CONTEXT *pContext, unsigned char *src, int src_w, int src_h, int min_face_sz, int max_face_sz, int rotation)
{
	FD_INTERNAL_RESULT track_results[FD_MAX_FIND];
	if(min_face_sz == DEFAULT_SIZE) min_face_sz = FD_MAX(FD_MIN(src_w/20, src_h/20), FD_FACE_WIDTH);
	if(max_face_sz == DEFAULT_SIZE) max_face_sz = FD_MIN(src_w, src_h);

	int normal_rip_sequence[] = {ROTATE_TOP_UP, ROTATE_LEFT_UP, ROTATE_RIGHT_UP, ROTATE_BOTTOM_UP};
	int sdic_rip_sequence[] = {ROTATE_TOP_UP, ROTATE_LEFT_UP, ROTATE_TOP_UP, ROTATE_RIGHT_UP};
	int rip_sequence[4];
	if(rotation == ROTATE_SDIC_SEARCH) fd_memcpy(rip_sequence, sdic_rip_sequence, 4*sizeof(int));
	else fd_memcpy(rip_sequence, normal_rip_sequence, 4*sizeof(int));
	if(pContext->m_nFinalResultCnt < 20)
	{
		int track_face_num = MVFD_VideoTrackingFaces(pContext, src, src_w, src_h, pContext->m_FinalResults, pContext->m_nFinalResultCnt, rotation, false, track_results);

		//트래킹된 결과를 저장
		fd_memcpy(pContext->m_FinalResults, track_results, sizeof(FD_INTERNAL_RESULT)*track_face_num);
		pContext->m_nFinalResultCnt = track_face_num;

		//처음 시작시에는 frame_complete == ture 이기 때문에 무조건 처리됨
		if(pContext->video_info.frame_complete)
		{
			int new_face_num = 0;			
			FD_INTERNAL_RESULT detect_results[FD_MAX_FIND];
			FD_INTERNAL_RESULT detect_results2[FD_MAX_FIND];
			int detect_face_num = MVFD_BoxClustering(&pContext->detect_enroll_faces, FD_MIN_INTERSECTION_CNT, detect_results);
			
			if(detect_face_num > 0)
			{
				if(pContext->m_nFinalResultCnt > 0)
				{
					//검출된 얼굴들에서 트래킹에서 잡은 얼굴들을 제거함
					int remain_face_num = MVFD_RemoveOverlappedFaces(detect_results, detect_face_num, track_results, track_face_num);

					if(remain_face_num > 0)
					{
						//트래킹된 얼굴들이 제거된 검출 결과에서 실제 얼굴이 존재하는지 확인 (2차 트래킹)
						int detect_face_num2 = MVFD_VideoTrackingFaces(pContext, src, src_w, src_h, detect_results, remain_face_num, rotation, true, detect_results2);
						//2차 트래킹을 통해 존재가 확인된 얼굴들 중에 기존 트래킹 결과와 겹치는 것이 있으면 제거
						pContext->m_nFinalResultCnt = MVFD_MergeFaces_UseConfidence(detect_results2, detect_face_num2, track_results, track_face_num, pContext->m_FinalResults);
					}
				}
				else
				{
					new_face_num = detect_face_num;
					//최종 결과에 새로 검출된 얼굴들을 추가
					if(track_face_num+new_face_num > FD_MAX_FIND) new_face_num = FD_MAX_FIND-track_face_num;
					fd_memcpy(&pContext->m_FinalResults[track_face_num], detect_results, sizeof(FD_INTERNAL_RESULT)*new_face_num);
					pContext->m_nFinalResultCnt += new_face_num;
				}
			}

			//다음에 처리할 새 프레임을 넣음
			FD_CONTEXT::VIDEO_INFO &vi = pContext->video_info;
			vi.video_frame = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_VIDEO_KEY_FRAME, src_w*src_h);
			memcpy(vi.video_frame, src, src_w*src_h);
			vi.frame_width = src_w;
			vi.frame_height = src_h;
			vi.min_face_sz = min_face_sz;
			vi.max_face_sz = max_face_sz;
			vi.frame_complete = false;
			if(rotation < 4)
			{
				vi.rip_idx = 0;
				vi.orientation = rotation;
			}
			else
			{
				if(detect_face_num == 0) 
				{
					if(rotation == ROTATE_180_SEARCH) vi.rip_idx = (vi.rip_idx+1)%3;
					else if(rotation == ROTATE_FULL_SEARCH || rotation == ROTATE_SDIC_SEARCH) vi.rip_idx = (vi.rip_idx+1)%4;
					vi.orientation = rip_sequence[vi.rip_idx];
				}
			}
			MVFD_Mutex_UnLock(&vi.frame_recv_mutex);
		}
	}
	else
	{
		if(pContext->video_info.frame_complete)
		{
			FD_RECT roi = {0, 0, src_w, src_h};
			MVFD_DetectFace_Image(pContext, src, src_w, src_h, roi, min_face_sz, max_face_sz, rotation);
		}
	}
	return pContext->m_nFinalResultCnt;
}

void *MVFD_Video_Detection_Thread(void *pArg)
{
	FD_CONTEXT *pContext = (FD_CONTEXT *)pArg;
	FD_CONTEXT::VIDEO_INFO &vi = pContext->video_info;
	while(1)
	{
		MVFD_Mutex_Lock(&vi.frame_recv_mutex);
		if(pContext->wait_for_exit) break;
		
		MVFD_ClearFaceEnrollStorage(&pContext->detect_enroll_faces);
		
		int rot_src_w, rot_src_h;
		unsigned char *rot_src;
		if(vi.orientation == ROTATE_TOP_UP) rot_src = vi.video_frame;
		else 
		{
			rot_src = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_ROTATION_IMG, vi.frame_width*vi.frame_height);
			MVFD_MT_Rotate_RIP(vi.video_frame, vi.frame_width, vi.frame_height, rot_src, vi.orientation, 0, vi.frame_height);
		}
		if(vi.orientation == ROTATE_TOP_UP || vi.orientation == ROTATE_BOTTOM_UP)
		{
			rot_src_w = vi.frame_width;
			rot_src_h = vi.frame_height;
		}
		if(vi.orientation == ROTATE_LEFT_UP || vi.orientation == ROTATE_RIGHT_UP)
		{
			rot_src_w = vi.frame_height;
			rot_src_h = vi.frame_width;
		}

		MVFD_Run_Detector_Threads(pContext, rot_src, rot_src_w, rot_src_h, 
								  vi.orientation, vi.min_face_sz, vi.max_face_sz, FD_VIDEO_SCALE_RATE);		
		vi.frame_complete = true;
	}
	return NULL;
}

int MVFD_VideoTrackingFaces(FD_CONTEXT *pContext, unsigned char *src, int src_w, int src_h, FD_INTERNAL_RESULT *prev_results, int prev_results_num, int rotation, bool half_margin, FD_INTERNAL_RESULT *track_results)
{
	static const int FD_MAX_PATCH_NUM = 1000;
	IMG_PATCH_INFO patch[FD_MAX_PATCH_NUM];
	FD_INTERNAL_RESULT temp_track_results[FD_MAX_PATCH_NUM];

	int patch_rip[FD_MAX_PATCH_NUM];
	int patch_trycnt[FD_MAX_PATCH_NUM] = {0,};
	int patch_found[FD_MAX_PATCH_NUM] = {0,};
	int patch_found_cnt = 0;
	FD_RECT patch_roi[FD_MAX_PATCH_NUM];
	
	int total_face_cnt = 0;
	FD_POINT prev_face_xy[FD_MAX_PATCH_NUM];
	int patch_num = 0;
	int largest_patch_idx = 0, largest_patch_idx2 = 0;
	const float scale[] = {1.0, 0.9, 1.11};	
	
	if(prev_results_num == 0) return 0;

#ifdef USE_NEON
	unsigned int *scratchBuffer = MVFD_MemSection_Alloc<unsigned int>(pContext, MEM_SECTION_ISFACE_SCRATCH_BUFFER, 300*6);//Max feature num is < 300		
#endif

	for(int i = 0; i < prev_results_num; ++i)
	{
		int face_sz = prev_results[i].rect.right-prev_results[i].rect.left;
		float face_scale = FD_FACE_WIDTH/double(face_sz);
		int side_margin_w = half_margin ? face_sz/2 : face_sz;
		int side_margin_h = half_margin ? face_sz/2 : face_sz;
		FD_RECT roi;
		roi.left = prev_results[i].rect.left-side_margin_w;
		roi.top  = prev_results[i].rect.top-side_margin_h;
		roi.right = prev_results[i].rect.right+side_margin_w;
		roi.bottom = prev_results[i].rect.bottom+side_margin_h;

		patch_roi[i] = roi;
		if(patch_num+3 < FD_MAX_PATCH_NUM)
		{
			for(int j = 0; j < 3; ++j)
			{
				patch_rip[patch_num] = prev_results[i].img_direction;
				patch[patch_num].roi = roi;
				patch[patch_num].resize_w = (roi.right-roi.left)*face_scale*scale[j];
				patch[patch_num].resize_h = (roi.bottom-roi.top)*face_scale*scale[j];
				prev_face_xy[patch_num].x = (prev_results[i].rect.left-roi.left)*face_scale*scale[j];
				prev_face_xy[patch_num].y = (prev_results[i].rect.top-roi.top)*face_scale*scale[j];

				if(patch[largest_patch_idx].resize_w*patch[largest_patch_idx].resize_h < patch[patch_num].resize_w*patch[patch_num].resize_h) largest_patch_idx = patch_num;
				int largest_patch_size2 = FD_MAX(IRMB2_ALIGNED_WIDTH(patch[largest_patch_idx2].resize_w)*patch[largest_patch_idx2].resize_h,
												 IRMB2_ALIGNED_WIDTH(patch[largest_patch_idx2].resize_h)*patch[largest_patch_idx2].resize_w);
				int cur_patch_size2		= FD_MAX(IRMB2_ALIGNED_WIDTH(patch[patch_num].resize_w)*patch[patch_num].resize_h,
												 IRMB2_ALIGNED_WIDTH(patch[patch_num].resize_h)*patch[patch_num].resize_w);
				if(largest_patch_size2 < cur_patch_size2) largest_patch_idx2 = patch_num;
				patch_num++;
			}
		}
	}

	int feature_buf_sz = 0;
	feature_buf_sz = patch[largest_patch_idx].resize_w*patch[largest_patch_idx].resize_h*4;
	feature_buf_sz = ALIGN16(feature_buf_sz) + 
					 FD_MAX(IRMB2_ALIGNED_WIDTH(patch[largest_patch_idx2].resize_w)*patch[largest_patch_idx2].resize_h*4,
							IRMB2_ALIGNED_WIDTH(patch[largest_patch_idx2].resize_h)*patch[largest_patch_idx2].resize_w*4);

	unsigned char *feature_buf = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_FEATURE_BUF, feature_buf_sz);
	unsigned short *pose_map = MVFD_MemSection_Alloc<unsigned short>(pContext, MEM_SECTION_POSE_MAP, patch[largest_patch_idx].resize_w*patch[largest_patch_idx].resize_h);

	for(int try_cnt = 0; try_cnt < 4; try_cnt++)
	{
		MVFD_ClearFaceEnrollStorage(&pContext->track_enroll_faces); 
		for(int i = 0; i < prev_results_num; i++)
		{
			if(patch_found[i]) continue;
			int found_face_num = 0;
			patch_trycnt[i]++;
			for(int j = 0; j < 3; ++j)
			{
				int pidx = i*3+j;
				int patch_sz = patch[pidx].resize_w*patch[pidx].resize_h;
				patch[pidx].feature.img = feature_buf;
				patch[pidx].feature.rot = &feature_buf[patch_sz];
				patch[pidx].feature.mct = (unsigned short *)&feature_buf[patch_sz*2];
				patch[pidx].feature.irmb = &feature_buf[ALIGN16(patch_sz*4)];	//16byte align된 메모리
		
				unsigned char *resize_buf = MVFD_MemSection_Alloc<unsigned char>(pContext, MEM_SECTION_RESIZE_BUF, patch[pidx].resize_w*patch[pidx].resize_h);				
				FD_RECT dst_roi = {0, 0, patch[pidx].resize_w, patch[pidx].resize_h};
				if(patch[pidx].roi.left < 0 || patch[pidx].roi.top < 0 || patch[pidx].roi.right >= src_w || patch[pidx].roi.bottom >= src_h)
				{
					memset(resize_buf, 0, patch[pidx].resize_w*patch[pidx].resize_h);
				}
				if(!MVFD_MT_ResizeBI_R2R(src, src_w, src_h, patch[pidx].roi, resize_buf, patch[pidx].resize_w, patch[pidx].resize_h, dst_roi)) continue;
				
				if(patch_rip[pidx] == ROTATE_TOP_UP) patch[pidx].feature.img = resize_buf;
				else 
				{
					MVFD_MT_Rotate_RIP(resize_buf, patch[pidx].resize_w, patch[pidx].resize_h, patch[pidx].feature.img, patch_rip[pidx], 0, patch[pidx].resize_h);
					if(patch_rip[pidx] == ROTATE_LEFT_UP || patch_rip[pidx] == ROTATE_RIGHT_UP) 
					{
						if(patch_rip[pidx] == ROTATE_LEFT_UP)
						{
							prev_face_xy[pidx].x = patch[pidx].resize_h-(prev_face_xy[pidx].y+FD_FACE_WIDTH);
							prev_face_xy[pidx].y = prev_face_xy[pidx].x;
						}
						if(patch_rip[j] == ROTATE_RIGHT_UP)
						{
							prev_face_xy[pidx].x = prev_face_xy[pidx].y;
							prev_face_xy[pidx].y = patch[pidx].resize_w-(prev_face_xy[pidx].x+FD_FACE_HEIGHT);
						}
						if(patch_rip[j] == ROTATE_BOTTOM_UP)
						{
							prev_face_xy[pidx].y = patch[pidx].resize_h-(prev_face_xy[pidx].y+FD_FACE_HEIGHT);
						}
						int temp = patch[pidx].resize_w;
						patch[pidx].resize_w = patch[pidx].resize_h;
						patch[pidx].resize_h = temp;
					}
				}
				patch[pidx].feature.width = patch[pidx].resize_w;
				patch[pidx].feature.height = patch[pidx].resize_h;

#ifndef USE_NEON
				MVFD_ImageRotate(patch[pidx].feature.img, patch[pidx].resize_w, patch[pidx].resize_h, patch[pidx].feature.rot);
#else
				MVFD_ImageTranspose(patch[pidx].feature.img, patch[pidx].resize_w, patch[pidx].resize_h, patch[pidx].feature.rot);
#endif
				MVFD_MT_Image2MCT(patch[pidx].feature.img, patch[pidx].resize_w, patch[pidx].resize_h, patch[pidx].feature.mct, 0, patch[pidx].resize_h);
				MVFD_Image2IRMB(patch[pidx].feature.img, patch[pidx].resize_w, patch[pidx].resize_h, pContext, patch[pidx].feature.irmb);

				//showimg(patch[pidx].feature.img, patch[pidx].resize_w, patch[pidx].resize_h);
				
				for(int k = 1; k <= 9; ++k)
				{
					for(int l = 0; l < 10; ++l)
					{
						if(MVFD_Detector[k][l]) MVFD_RescaleFeaturePositions(pContext, MVFD_Detector[k][l], patch[pidx].resize_w, patch[pidx].resize_h, pContext->trk_ffpos.face[k][l]);
					}
				}
				MVFD_RescaleFeaturePositions(pContext, MVFD_PoseClassifier, patch[pidx].resize_w, patch[pidx].resize_h, pContext->trk_ffpos.pose_map);
		
				MVFD_GeneratePoseMapForTracking(pContext, MVFD_BkgndRejector, MVFD_PoseClassifier, &patch[pidx].feature, FD_TRACKING_REJECTION_RATE, pose_map, prev_face_xy[pidx].x, prev_face_xy[pidx].y, pContext->trk_ffpos.pose_map);
				//MVFD_GeneratePoseMap(MVFD_BkgndRejector, MVFD_PoseClassifier, &patch[pidx].feature, FD_TRACKING_REJECTION_RATE, pose_map);
		
				for(int y = 0; y < patch[pidx].resize_h-FD_FACE_HEIGHT; y +=2)
				{
					for(int x = 0; x < patch[pidx].resize_w-FD_FACE_WIDTH; x++)
					{
#ifdef FD_NO_REJECTOR_TO_FRONTAL_FACE
						int confidence;
#ifdef USE_NEON
						confidence = MVFD_IsFace(&patch[pidx].feature, x, y, MVFD_Detector[9][0], scratchBuffer, pContext->trk_ffpos.face[9]);
#else
						confidence = MVFD_IsFace(&patch[pidx].feature, x, y, MVFD_Detector[9][0], pContext->trk_ffpos.face[9]);
#endif
						if(confidence )
						{
							found_face_num++;
							float inv_scale_rate = (patch_rip[pidx] == ROTATE_LEFT_UP || patch_rip[pidx] == ROTATE_RIGHT_UP)  ? 
													(patch[pidx].roi.bottom-patch[pidx].roi.top)/float(patch[pidx].resize_h) :
													(patch[pidx].roi.right-patch[pidx].roi.left)/float(patch[pidx].resize_w);
							MVFD_EnrollFace2(x, y, inv_scale_rate, patch[pidx].resize_w, patch[pidx].resize_h, patch[pidx].roi.left, patch[pidx].roi.top,
										9, patch_rip[pidx], confidence, &pContext->track_enroll_faces);
							continue;
						}
#endif
						int pose = pose_map[y*patch[pidx].resize_w+x];
						if(pose != 0xffff)
						{
#ifndef FD_NO_REJECTOR_TO_FRONTAL_FACE
							//frontal face detector is directly applied, regardless of result of the pose classifier.
							int confidence;
#ifdef USE_NEON
							confidence  = MVFD_IsFace(&patch[pidx].feature, x, y, MVFD_Detector[9][0], scratchBuffer, pContext->trk_ffpos.face[9]);
#else
							confidence  = MVFD_IsFace(&patch[pidx].feature, x, y, MVFD_Detector[9][0], pContext->trk_ffpos.face[9]);
#endif
							if(confidence)
							{
								found_face_num++;
								float inv_scale_rate = (patch_rip[pidx] == ROTATE_LEFT_UP || patch_rip[pidx] == ROTATE_RIGHT_UP)  ? 
														(patch[pidx].roi.bottom-patch[pidx].roi.top)/float(patch[pidx].resize_h) :
														(patch[pidx].roi.right-patch[pidx].roi.left)/float(patch[pidx].resize_w) ;	
								MVFD_EnrollFace2(x, y, inv_scale_rate, patch[pidx].resize_w, patch[pidx].resize_h, patch[pidx].roi.left, patch[pidx].roi.top,
											9, patch_rip[pidx], confidence, &pContext->track_enroll_faces);
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
									confidence = MVFD_IsFace(&patch[pidx].feature, x, y, MVFD_Detector[pose1][0], scratchBuffer, pContext->trk_ffpos.face[pose1]);
#else
									confidence = MVFD_IsFace(&patch[pidx].feature, x, y, MVFD_Detector[pose1][0], pContext->trk_ffpos.face[pose1]);
#endif
									if(confidence)
									{
										found_face_num++;
										float inv_scale_rate = (patch_rip[pidx] == ROTATE_LEFT_UP || patch_rip[pidx] == ROTATE_RIGHT_UP)  ? 
															   (patch[pidx].roi.bottom-patch[pidx].roi.top)/float(patch[pidx].resize_h) :
															   (patch[pidx].roi.right-patch[pidx].roi.left)/float(patch[pidx].resize_w);								
										MVFD_EnrollFace2(x, y, inv_scale_rate, patch[pidx].resize_w, patch[pidx].resize_h, patch[pidx].roi.left, patch[pidx].roi.top,
													pose1, patch_rip[pidx], confidence, &pContext->track_enroll_faces);
									}
								}
					
								if(MVFD_Detector[pose2][0])
								{
#ifdef USE_NEON
									confidence = MVFD_IsFace(&patch[pidx].feature, x, y, MVFD_Detector[pose2][0], scratchBuffer, pContext->trk_ffpos.face[pose2]);
#else
									confidence = MVFD_IsFace(&patch[pidx].feature, x, y, MVFD_Detector[pose2][0], pContext->trk_ffpos.face[pose2]);
#endif
									if(confidence)
									{
										found_face_num++;
										float inv_scale_rate = (patch_rip[pidx] == ROTATE_LEFT_UP || patch_rip[pidx] == ROTATE_RIGHT_UP)  ? 
															   (patch[pidx].roi.bottom-patch[pidx].roi.top)/float(patch[pidx].resize_h) :
															   (patch[pidx].roi.right-patch[pidx].roi.left)/float(patch[pidx].resize_w);
										MVFD_EnrollFace2(x, y, inv_scale_rate, patch[pidx].resize_w, patch[pidx].resize_h, patch[pidx].roi.left, patch[pidx].roi.top,
													pose2, patch_rip[pidx], confidence, &pContext->track_enroll_faces);
									}
								}
							}
						}
					}
				}
#ifndef FD_TRACKING_TRY_3SCALES
				if(found_face_num > 3) break;
#endif
			}
		}
		int face_cnt = MVFD_BoxClustering(&pContext->track_enroll_faces, FD_MIN_INTERSECTION_CNT, temp_track_results);
		fd_memcpy(&track_results[total_face_cnt], &temp_track_results, face_cnt*sizeof(FD_INTERNAL_RESULT));
		total_face_cnt += face_cnt;

		if(rotation == ROTATE_180_SEARCH || rotation == ROTATE_FULL_SEARCH || rotation == ROTATE_SDIC_SEARCH)
		{
			bool rip_search_done = true;
			for(int i = 0; i < prev_results_num; ++i)
			{
				if((rotation == ROTATE_180_SEARCH && patch_trycnt[i] < 3) || 
					(rotation == ROTATE_FULL_SEARCH && patch_trycnt[i] < 4) ||
					(rotation == ROTATE_SDIC_SEARCH && patch_trycnt[i] < 3)) rip_search_done = false;
				if(!patch_found[i])
				{
					for(int j = 0; j < face_cnt; ++j)
					{
						if(patch_roi[i].left < track_results[j].rect.left && track_results[j].rect.right < patch_roi[i].right &&
							patch_roi[i].top < track_results[j].rect.top && track_results[j].rect.bottom < patch_roi[i].bottom)
						{
							patch_found[i] = true; 
							patch_found_cnt++;
							break;
						}
					}
					if(!patch_found[i])
					{
						if(rotation == ROTATE_180_SEARCH) patch_rip[i] = (patch_rip[i]+1)%3;
						else if(rotation == ROTATE_FULL_SEARCH) patch_rip[i] = (patch_rip[i]+1)%4;
						else if(rotation == ROTATE_SDIC_SEARCH) patch_rip[i] = (patch_rip[i]+1)%3;
					}
				}
			}
			if(patch_found_cnt == prev_results_num || rip_search_done) break;
		}
		else break;
	}
	return total_face_cnt;
}

int MVFD_RemoveOverlappedFaces(FD_INTERNAL_RESULT src_faces[], int src_num, FD_INTERNAL_RESULT remove_faces[], int remove_num)
{
	int i, j;
	int remain_num = 0;
	for(i = 0; i < src_num; ++i)
	{
		for(j = 0; j < remove_num; ++j)
		{
			FD_RECT &rc1 = src_faces[i].rect;
			FD_RECT &rc2 = remove_faces[j].rect;

			if((rc1.left-rc2.right)*(rc1.right-rc2.left) < 0 && (rc1.top-rc2.bottom)*(rc1.bottom-rc2.top) < 0)
			{
				if((FD_MIN(rc1.right, rc2.right)-FD_MAX(rc1.left, rc2.left))*(FD_MIN(rc1.bottom, rc2.bottom)-FD_MAX(rc1.top, rc2.top)) > 
					FD_MIN((rc1.right-rc1.left)*(rc1.bottom-rc1.top), (rc2.right-rc2.left)*(rc2.bottom-rc2.top))>>1)
				{
					break;
				}
			}
		}
		if(j != remove_num)
		{
			if(FD_MIN(src_faces[i].intersect_cnt, remove_faces[j].intersect_cnt) < 5 &&
				abs(src_faces[i].intersect_cnt-remove_faces[j].intersect_cnt) > 2)
			{
				if(src_faces[i].intersect_cnt > remove_faces[j].intersect_cnt) 
					fd_memcpy(&src_faces[remain_num++], &src_faces[i], sizeof(FD_INTERNAL_RESULT));
				else
					fd_memcpy(&src_faces[remain_num++], &remove_faces[j], sizeof(FD_INTERNAL_RESULT));
			}
			else
			{
				if(src_faces[i].likelihood > remove_faces[j].likelihood) 
					fd_memcpy(&src_faces[remain_num++], &src_faces[i], sizeof(FD_INTERNAL_RESULT));
				else
					fd_memcpy(&src_faces[remain_num++], &remove_faces[j], sizeof(FD_INTERNAL_RESULT));
			}
		}
		else fd_memmove(&src_faces[remain_num++], &src_faces[i], sizeof(FD_INTERNAL_RESULT));
	}
	return remain_num;
}

int MVFD_MergeFaces_UseConfidence(FD_INTERNAL_RESULT src_faces1[], int src_num1, FD_INTERNAL_RESULT src_faces2[], int src_num2,  FD_INTERNAL_RESULT new_faces[])
{
	int i, j;
	int remain_num = 0;
	bool found[300] = {false,};
	for(i = 0; i < src_num1; ++i)
	{
		for(j = 0; j < src_num2; ++j)
		{
			FD_RECT &rc1 = src_faces1[i].rect;
			FD_RECT &rc2 = src_faces2[j].rect;

			if((rc1.left-rc2.right)*(rc1.right-rc2.left) < 0 && (rc1.top-rc2.bottom)*(rc1.bottom-rc2.top) < 0)
			{
				if((FD_MIN(rc1.right, rc2.right)-FD_MAX(rc1.left, rc2.left))*(FD_MIN(rc1.bottom, rc2.bottom)-FD_MAX(rc1.top, rc2.top)) > 
					FD_MIN((rc1.right-rc1.left)*(rc1.bottom-rc1.top), (rc2.right-rc2.left)*(rc2.bottom-rc2.top))>>1)
				{
					found[j] = true;
					break;
				}
			}
		}
		if(j == src_num2)
		{
			fd_memcpy(&new_faces[remain_num++], &src_faces1[i], sizeof(FD_INTERNAL_RESULT));
		}
		else
		{
			if(FD_MIN(src_faces1[i].intersect_cnt, src_faces2[j].intersect_cnt) < 5 &&
				abs(src_faces1[i].intersect_cnt-src_faces2[j].intersect_cnt) > 2)
			{
				if(src_faces1[i].intersect_cnt > src_faces2[j].intersect_cnt) 
					fd_memcpy(&new_faces[remain_num++], &src_faces1[i], sizeof(FD_INTERNAL_RESULT));
				else
					fd_memcpy(&new_faces[remain_num++], &src_faces2[j], sizeof(FD_INTERNAL_RESULT));
			}
			else
			{
				if(src_faces1[i].likelihood > src_faces2[j].likelihood) 
					fd_memcpy(&new_faces[remain_num++], &src_faces1[i], sizeof(FD_INTERNAL_RESULT));
				else
					fd_memcpy(&new_faces[remain_num++], &src_faces2[j], sizeof(FD_INTERNAL_RESULT));
			}
		}
	}
	for(i = 0; i < src_num2; ++i)
	{
		if(!found[i]) {
			fd_memcpy(&new_faces[remain_num++], &src_faces2[i], sizeof(FD_INTERNAL_RESULT));
		}
	}
	return remain_num;
}

