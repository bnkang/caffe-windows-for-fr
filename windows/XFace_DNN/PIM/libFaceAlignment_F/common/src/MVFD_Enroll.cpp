//#include <stdlib.h>
//#include <memory.h>
#include "stdafx.h"

#include "MVFD_Defines.h"
#include "MVFD_Global.h"
#include "MVFD_Memory.h"
#define HASH_KEY_RANGE	10000

void MVFD_InitFaceEnrollStorage(FACE_ENROLL_STORAGE *storage)
{
	storage->FaceCandiHash = (int *)MVFD_mem_alloc(HASH_KEY_RANGE*sizeof(int));
	FDASSERT(storage->FaceCandiHash != NULL);
	storage->FaceCandiHashIdxs = (FACE_HASH_ELEMENT *)MVFD_mem_alloc(2000*sizeof(FACE_HASH_ELEMENT));
	FDASSERT(storage->FaceCandiHashIdxs != NULL);
	storage->FaceCandidates = (FD_INTERNAL_RESULT *)MVFD_mem_alloc(2000*sizeof(FD_INTERNAL_RESULT));
	FDASSERT(storage->FaceCandidates != NULL);
	
	for(int i = 0; i < HASH_KEY_RANGE; ++i) storage->FaceCandiHash[i] = -1;
	storage->nHashIdxNum = 0;
	storage->nHashIdxCapacity = 2000;
	storage->nFaceCandidateNum = 0;
	storage->nFaceCandidateCapacity = 2000;
}

void MVFD_FreeFaceEnrollStorage(FACE_ENROLL_STORAGE *storage)
{
	MVFD_mem_free(storage->FaceCandiHash);
	MVFD_mem_free(storage->FaceCandiHashIdxs);
	MVFD_mem_free(storage->FaceCandidates);
}

void MVFD_ClearFaceEnrollStorage(FACE_ENROLL_STORAGE *storage)
{
	storage->nFaceCandidateNum = 0;
	storage->nHashIdxNum = 0;
	for(int i = 0; i < HASH_KEY_RANGE; ++i) storage->FaceCandiHash[i] = -1;
}

void MVFD_EnrollFace(FACE_ENROLL_STORAGE *storage, FD_RECT rect, int pose, int img_direction, int likelihood)
{
	bool found = false;
	int hash_key = ((int(abs(rect.top))<<18)+(int(abs(rect.left))<<12)+(int(abs(rect.right))<<6)+int(abs(rect.bottom))) % HASH_KEY_RANGE;
	int idx_pos = storage->FaceCandiHash[hash_key];
	if(idx_pos != -1)
	{
		while(1)
		{
			FD_INTERNAL_RESULT &exist_face = storage->FaceCandidates[storage->FaceCandiHashIdxs[idx_pos].pos];
			FD_RECT &hrect = exist_face.rect;
			if(hrect.left == rect.left && hrect.top == rect.top && hrect.right == rect.right && hrect.bottom == rect.bottom && exist_face.img_direction == img_direction)
			{
				if(exist_face.likelihood < likelihood)
				{
					exist_face.likelihood = likelihood;
					exist_face.pose = pose;			
				}
				found = true;
				break;
			}
			int next_idx_pos = storage->FaceCandiHashIdxs[idx_pos].next;
			if(next_idx_pos == -1) break;
			idx_pos = next_idx_pos;
		}
	}

	if(!found)
	{
		if(storage->nFaceCandidateNum < storage->nFaceCandidateCapacity)
		{
			storage->FaceCandidates[storage->nFaceCandidateNum].rect   = rect;
			storage->FaceCandidates[storage->nFaceCandidateNum].img_direction = img_direction;
			storage->FaceCandidates[storage->nFaceCandidateNum].likelihood = likelihood;
			storage->FaceCandidates[storage->nFaceCandidateNum].pose = pose;
			storage->FaceCandiHashIdxs[storage->nHashIdxNum].pos  = storage->nFaceCandidateNum;
			storage->FaceCandiHashIdxs[storage->nHashIdxNum].next = -1;

			if(idx_pos == -1) storage->FaceCandiHash[hash_key] = storage->nHashIdxNum;
			else storage->FaceCandiHashIdxs[idx_pos].next = storage->nHashIdxNum;
			
			storage->nFaceCandidateNum++;
			storage->nHashIdxNum++;
		}
		else
		{
			storage->nFaceCandidateCapacity *= 2;
			FD_INTERNAL_RESULT *new_buffer1 = (FD_INTERNAL_RESULT *)MVFD_mem_alloc(storage->nFaceCandidateCapacity*sizeof(FD_INTERNAL_RESULT));
			FDASSERT(new_buffer1 != NULL);
			fd_memcpy(new_buffer1, storage->FaceCandidates, sizeof(FD_INTERNAL_RESULT)*storage->nFaceCandidateNum);
			MVFD_mem_free(storage->FaceCandidates);
			storage->FaceCandidates = new_buffer1;

			storage->nHashIdxCapacity *= 2;
			FACE_HASH_ELEMENT *new_buffer2 = (FACE_HASH_ELEMENT *)MVFD_mem_alloc(storage->nHashIdxCapacity*sizeof(FACE_HASH_ELEMENT));
			FDASSERT(new_buffer2 != NULL);
			fd_memcpy(new_buffer2, storage->FaceCandiHashIdxs, sizeof(FACE_HASH_ELEMENT)*storage->nHashIdxNum);
			MVFD_mem_free(storage->FaceCandiHashIdxs);
			storage->FaceCandiHashIdxs = new_buffer2;
		}
	}
}

void MVFD_EnrollFace2(int x, int y, float inv_scale_rate, int resize_w, int resize_h, int roi_left, int roi_top, int pose_idx, int rotation, int confidence, FACE_ENROLL_STORAGE *storage)
{
	FD_RECT rc1, rc2;
	rc1.left = x*inv_scale_rate+0.5;
	rc1.top  = y*inv_scale_rate+0.5;
	rc1.right = (x+FD_FACE_WIDTH)*inv_scale_rate+0.5;
	rc1.bottom = (y+FD_FACE_HEIGHT)*inv_scale_rate+0.5;
	
	if(rotation == ROTATE_LEFT_UP || rotation == ROTATE_RIGHT_UP)
	{
		MVFD_RotateRectToTopUp(&rc1, &rc2, resize_h*inv_scale_rate+0.5, resize_w*inv_scale_rate+0.5, rotation);
	}
	else if(rotation == ROTATE_BOTTOM_UP)
	{
		MVFD_RotateRectToTopUp(&rc1, &rc2, resize_w*inv_scale_rate+0.5, resize_h*inv_scale_rate+0.5, rotation);
	}
	else rc2 = rc1;
	
	rc2.left += roi_left;
	rc2.right += roi_left;
	rc2.top += roi_top;
	rc2.bottom += roi_top;

	FD_CLASSIFIER *pNode = MVFD_Detector[pose_idx][MVFDCascadeNum[pose_idx]-1];

	int bin_idx = FD_MIN(FD_MAX((confidence-int(pNode->fv_lower_new))/double(pNode->fv_upper_new-pNode->fv_lower_new),0.0), 0.999)*FD_NEW_LIKELIHOOD_BIN_NUM;
	int likelihood = pNode->likelihood_new[bin_idx];

	if(likelihood > MVFD_enroll_thresholds[pose_idx-1])
	{
		MVFD_EnrollFace(storage, rc2, pose_idx, rotation, likelihood);
	}
}

int MVFD_BoxClustering(FACE_ENROLL_STORAGE *storage, int min_intersection_cnt, FD_INTERNAL_RESULT *final_result)
{
	int cluster_cnt = 0;
	int stack_top = 0;
	int box_cnt = 0;
	int temp, seed;
	int i, j;
	int max_pose = 0, max_img_direction = 0;
	int divcnt = 0;
	int overlap_cnt;
	int max_idx;
	int	temp_result_cnt = 0;
	int result_cnt = 0;
	int pose_vote[FD_POSE_NUM+1];
	int img_direction_vote[4];
	int overlap_idx[FD_MAX_FIND];
	int valid[FD_MAX_FIND];
	int within_range_cnt = 0;
	FD_INTERNAL_RESULT temp_result[FD_MAX_FIND];
	
	int face_candi_num = storage->nFaceCandidateNum;
	if(face_candi_num == 0) return 0;

	int *large_buffer = (int *)MVFD_mem_alloc(face_candi_num*4*sizeof(int));
	int *cluster_idx = &large_buffer[0];
	int *visited = &large_buffer[face_candi_num];
	int *stack = &large_buffer[face_candi_num*2];
	int *rank_confidence = &large_buffer[face_candi_num*3];
	
	fd_memset(pose_vote, 0, FD_POSE_NUM*sizeof(int));
	fd_memset(visited, 0, face_candi_num*sizeof(int));
	
	for(i = 0; i < face_candi_num; ++i) cluster_idx[i] = -1;
	while(1)
	{
		seed = -1;
		for(i = 0; i < face_candi_num; ++i)
		{
			if(!visited[i]) 
			{
				seed = i;
				break;
			}
		}
		if(seed == -1) break;
		visited[seed] = 1;
		cluster_idx[seed] = cluster_cnt;
		//seed 주위에 있는 후보들을 추출
		within_range_cnt = 0;
		for(i = 0; i < face_candi_num; ++i)
		{
			if(!visited[i])
			{
				FD_RECT &rc1 = storage->FaceCandidates[seed].rect;
				FD_RECT &rc2 = storage->FaceCandidates[i].rect;

				if((rc1.left-rc2.right)*(rc1.right-rc2.left) < 0 && (rc1.top-rc2.bottom)*(rc1.bottom-rc2.top) < 0)
				{
					if((FD_MIN(rc1.right, rc2.right)-FD_MAX(rc1.left, rc2.left))*(FD_MIN(rc1.bottom, rc2.bottom)-FD_MAX(rc1.top, rc2.top)) > 
						FD_MIN((rc1.right-rc1.left)*(rc1.bottom-rc1.top), (rc2.right-rc2.left)*(rc2.bottom-rc2.top))/3)
					{
						visited[i] = 1;
						cluster_idx[i] = cluster_cnt;
					}
				}
			}
		}

		cluster_cnt++;
		if(cluster_cnt >= FD_MAX_FIND) break;
	}
	if(final_result == NULL) return cluster_cnt;

	fd_memset(temp_result, 0, sizeof(FD_INTERNAL_RESULT)*FD_MAX_FIND);
	
	/////////////////// for Pose Estimation ////////////////////
	float pose_rip[] = {0, -30, -30, -30, 0, 0, 30, 30, 30, 0};
	float pose_rop[] = {0, -70, 0, 70, -70, 70, -70, 0, 70, 0};
	////////////////////////////////////////////////////////////
	temp_result_cnt = 0;
	for(i = 0; i < cluster_cnt; ++i)
	{
		box_cnt = 0;
		for(j = 0; j < face_candi_num; ++j)
		{
			if(cluster_idx[j] == i) rank_confidence[box_cnt++] = storage->FaceCandidates[j].likelihood;
		}

#ifdef USE_ONLY_HIGH_CONFIDENCES_FOR_BOX_CLUSTERING
		//sort
		for(j = 0; j < box_cnt; ++j)
		{
			for(k = j+1; k < box_cnt; ++k)
			{
				if(rank_confidence[k] < rank_confidence[j])
				{
					temp = rank_confidence[j];
					rank_confidence[j] = rank_confidence[k];
					rank_confidence[k] = temp;
				}
			}
		}
#endif
		if(box_cnt > min_intersection_cnt)
		{
			divcnt = 0;
			fd_memset(pose_vote, 0, sizeof(pose_vote));
			fd_memset(img_direction_vote, 0, sizeof(img_direction_vote));
			for(j = 0; j < face_candi_num; ++j)
			{
				if(cluster_idx[j] == i)
				{
#ifdef USE_ONLY_HIGH_CONFIDENCES_FOR_BOX_CLUSTERING
					if(storage->FaceCandidates[j].likelihood >= rank_confidence[box_cnt>>1])
#endif
					{
						pose_vote[storage->FaceCandidates[j].pose]++;
						img_direction_vote[storage->FaceCandidates[j].img_direction]++;
						temp_result[temp_result_cnt].rect.left += storage->FaceCandidates[j].rect.left;
						temp_result[temp_result_cnt].rect.top += storage->FaceCandidates[j].rect.top;
						temp_result[temp_result_cnt].rect.right += storage->FaceCandidates[j].rect.right;
						temp_result[temp_result_cnt].rect.bottom += storage->FaceCandidates[j].rect.bottom;
						temp_result[temp_result_cnt].likelihood += storage->FaceCandidates[j].likelihood;
						temp_result[temp_result_cnt].pose_likelihood[storage->FaceCandidates[j].pose-1] += storage->FaceCandidates[j].likelihood;
						divcnt++;
					}
					/////////////////// for Pose Estimation ////////////////////
					temp_result[temp_result_cnt].rip_angle += pose_rip[storage->FaceCandidates[j].pose];
					temp_result[temp_result_cnt].rop_angle += pose_rop[storage->FaceCandidates[j].pose];
					////////////////////////////////////////////////////////////
				}
			}
			if(divcnt > 0)
			{
				if(temp_result_cnt < FD_MAX_FIND)
				{
					temp_result[temp_result_cnt].rect.left /= divcnt;
					temp_result[temp_result_cnt].rect.top /= divcnt;
					temp_result[temp_result_cnt].rect.right /= divcnt;
					temp_result[temp_result_cnt].rect.bottom /= divcnt;
					temp_result[temp_result_cnt].likelihood /= divcnt;
					temp_result[temp_result_cnt].intersect_cnt = box_cnt;
					/////////////////// for Pose Estimation ////////////////////
					temp_result[temp_result_cnt].rip_angle /= box_cnt;
					temp_result[temp_result_cnt].rop_angle /= box_cnt;
					////////////////////////////////////////////////////////////

					max_pose = 0; max_img_direction = 0;
					for(j = 0; j < FD_POSE_NUM; ++j) if(pose_vote[j] > pose_vote[max_pose]) max_pose = j;
					for(j = 0; j < 4; ++j) if(img_direction_vote[j] > img_direction_vote[max_img_direction]) max_img_direction = j;
					temp_result[temp_result_cnt].pose = max_pose;
					temp_result[temp_result_cnt].img_direction = max_img_direction;
					temp_result_cnt++;
				}
			}
		}
	}

	for(i = 0; i < temp_result_cnt; ++i) valid[i] = true;
	
	for(i = 0; i < temp_result_cnt; ++i)
	{
		if(valid[i])
		{
			overlap_idx[0] = i;
			overlap_cnt = 1;
			for(j = 0; j < temp_result_cnt; ++j)
			{
				if(i != j && valid[j])
				{
					FD_RECT &rc1 = temp_result[i].rect;
					FD_RECT &rc2 = temp_result[j].rect;

					if((rc1.left-rc2.right)*(rc1.right-rc2.left) < 0 && (rc1.top-rc2.bottom)*(rc1.bottom-rc2.top) < 0)
					{
						if((FD_MIN(rc1.right, rc2.right)-FD_MAX(rc1.left, rc2.left))*(FD_MIN(rc1.bottom, rc2.bottom)-FD_MAX(rc1.top, rc2.top)) > 
							FD_MIN((rc1.right-rc1.left)*(rc1.bottom-rc1.top), (rc2.right-rc2.left)*(rc2.bottom-rc2.top))/3)
						{
							overlap_idx[overlap_cnt++] = j;
						}
					}
				}
			}

			max_idx = 0;
			for(j = 0; j < overlap_cnt; ++j)
			{
				FD_INTERNAL_RESULT &r1 = temp_result[overlap_idx[j]];
				FD_INTERNAL_RESULT &r2 = temp_result[overlap_idx[max_idx]];
				if(r1.intersect_cnt > r2.intersect_cnt) max_idx = j;
				else if(r1.intersect_cnt == r2.intersect_cnt)
				{
					if(r1.likelihood > r2.likelihood) max_idx = j;
				}
				else if(r1.intersect_cnt > 5 && r2.intersect_cnt > 5 && abs(r1.intersect_cnt-r2.intersect_cnt) <= 2)
				{
					if(r1.likelihood > r2.likelihood) max_idx = j;
				}
			}
			for(j = 0; j < overlap_cnt; ++j) 
			{
				if(j != max_idx) valid[overlap_idx[j]] = false;
			}
		}
	}
	
	for(i = 0; i < temp_result_cnt; ++i)
	{
		if(valid[i])
		{
			float score = 0;
			for(int j = 0; j < FD_POSE_NUM; ++j)
			{
				score += MVFD_pose_weights[j]*temp_result[i].pose_likelihood[j];
			}
			if(score > MVFD_merged_threshold)
			{
				fd_memcpy(&final_result[result_cnt], &temp_result[i], sizeof(FD_INTERNAL_RESULT));
				result_cnt++;
			}
		}
	}

	
	//show all detected boxes (not merged)
#if 0
	result_cnt = 0;
	for(i = 0; i < storage->nFaceCandidateNum; ++i)
	{
		fd_memcpy(&final_result[result_cnt++], &storage->FaceCandidates[i], sizeof(FD_INTERNAL_RESULT));
	}
#endif

	MVFD_mem_free(large_buffer);
	return result_cnt;
}

void MVFD_RemoveOverlappedResults(FD_RESULT *result)
{
	int overlap_cnt;
	int max_idx;
	int overlap_idx[FD_MAX_FIND];
	int valid[FD_MAX_FIND];
	
	for(int i = 0; i < result->face_num; ++i) valid[i] = true;
	for(int i = 0; i < result->face_num; ++i)
	{
		if(valid[i])
		{
			overlap_idx[0] = i;
			overlap_cnt = 1;
			for(int j = 0; j < result->face_num; ++j)
			{
				if(i != j && valid[j])
				{
					FD_RECT &rc1 = result->face[i].rect;
					FD_RECT &rc2 = result->face[j].rect;
					if((rc1.left-rc2.right)*(rc1.right-rc2.left) < 0 && (rc1.top-rc2.bottom)*(rc1.bottom-rc2.top) < 0)
					{
						if((FD_MIN(rc1.right, rc2.right)-FD_MAX(rc1.left, rc2.left))*(FD_MIN(rc1.bottom, rc2.bottom)-FD_MAX(rc1.top, rc2.top)) > 
							FD_MIN((rc1.right-rc1.left)*(rc1.bottom-rc1.top), (rc2.right-rc2.left)*(rc2.bottom-rc2.top))/3)
						{
							overlap_idx[overlap_cnt++] = j;
						}
					}
				}
			}

			max_idx = 0;
			for(int j = 0; j < overlap_cnt; ++j)
			{
				if(result->face[overlap_idx[j]].intersect_cnt > result->face[overlap_idx[max_idx]].intersect_cnt) max_idx = j;
			}
			for(int j = 0; j < overlap_cnt; ++j) 
			{
				if(j != max_idx) valid[overlap_idx[j]] = false;
			}
		}
	}
	
	int new_face_num = 0;
	int from_idx = 0;
	for(int i = 0; i < result->face_num; ++i)
	{
		while(from_idx < result->face_num && !valid[from_idx]) from_idx++;		
		if(from_idx == result->face_num) break;
		if(i != from_idx) fd_memcpy(&result->face[i], &result->face[from_idx], sizeof(FD_RESULT));
		from_idx++;
		new_face_num++;
	}
	result->face_num = new_face_num;
}

void MVFD_GetResults(FD_CONTEXT *pContext, FD_RESULT *results)
{
	if(results->overwrite) results->face_num = 0;
	/*
	for(int i = 0; i < g_detect_enroll_faces.nFaceCandidateNum; ++i)
	{
		results->face[results->face_num].rect = g_detect_enroll_faces.FaceCandidates[i].rect;
		results->face_num++;
	}
	*/

	for(int i = 0; i < pContext->m_nFinalResultCnt; ++i)
	{
		if(i == results->capacity) break;
		results->face[results->face_num].rect = pContext->m_FinalResults[i].rect;
		results->face[results->face_num].img_direction = pContext->m_FinalResults[i].img_direction;
		results->face[results->face_num].rip_angle = pContext->m_FinalResults[i].rip_angle;
		results->face[results->face_num].rop_angle = pContext->m_FinalResults[i].rop_angle;
		results->face[results->face_num].pose = pContext->m_FinalResults[i].pose;
		results->face[results->face_num].intersect_cnt = pContext->m_FinalResults[i].intersect_cnt;
		results->face[results->face_num].likelihood = pContext->m_FinalResults[i].likelihood;
		results->face_num++;
	}
}


void MVFD_AllocResult(FD_RESULT *result, int max_face_num)
{
	result->face = (ONE_FACE *)MVFD_mem_alloc(sizeof(ONE_FACE)*max_face_num);
	result->face_num = 0;
	result->overwrite = true;
	result->capacity = max_face_num;
}

void MVFD_FreeResult(FD_RESULT *result)
{
	MVFD_mem_free(result->face);
}
