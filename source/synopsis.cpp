#include "synopsis.h"
#include <io.h>

/**
 *������ģ
 */
void bgModeling(const char * videoFilePath, const int frame_num_used, IplImage ** bgImg, \
	const int size1, const int size2, const int sigma1, const int sigma2){
	//����
	IplImage * frame = NULL;
	CvMat * frameMat = NULL;
	CvMat * bgMat = NULL;
	CvCapture* pCapture = NULL;
	int frame_no = 0;

	pCapture = cvCaptureFromFile(videoFilePath);//�Լ�ѡȡһ��avi��Ƶ
	if(!pCapture){
		printf("Unable to open video file for background modeling!\n");
		return;
	}

	if(*bgImg != NULL){//�ǿ��������*bgImgָ����ڴ�
		cvReleaseImage(bgImg);
	}

	printf("Background Modeling...\n");
	//��֡��ȡ��Ƶ
	while(frame_no < frame_num_used){
		
		frame = cvQueryFrame(pCapture);
		frame_no += 1;

		if(frame_no == 1){
			//��ʼ��
			*bgImg = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, frame->nChannels);
			cvCopy(frame, *bgImg);
			frameMat = cvCreateMat(frame->height, frame->width, CV_32FC3);
			bgMat = cvCreateMat((*bgImg)->height, (*bgImg)->width, CV_32FC3);
			cvConvert(frame, frameMat);
			cvConvert(*bgImg, bgMat);
			continue;
		}
		//��Ƶ֡IplImageתCvMat
		cvConvert(frame, frameMat);
		//��˹�˲��ȣ���ƽ��ͼ��
		cvSmooth(frame, frame, CV_GAUSSIAN, size1, size2, sigma1, sigma2);
		//����ƽ�����±���(��ƽ��)
		cvRunningAvg(frameMat, bgMat, (double)1/frame_num_used);

	}
	cvConvert(bgMat, *bgImg);
	printf("Background Model has been achieved!\n");

	//�ͷ��ڴ�
	cvReleaseCapture(&pCapture);
	cvReleaseMat(&frameMat);
	cvReleaseMat(&bgMat);
}


/**
 *�������ݵ�Ԫ���캯��
 */
tube::tube(CvRect rect, int t, IplImage * img):position(rect), t_sec(t){ 
	target = cvCreateImage(cvSize(img->width, img->height), img->depth, img->nChannels);
	cvCopy(img, target);
}

/**
 *�������ݵ�Ԫ��������
 */
tube::~tube(){
	cvReleaseImage(&target);
}


/**
 *��ȡǰ��ͼ�� - ��ֵͼ��
 */
void getFgImage(const IplImage * frame, IplImage * fgImg, const IplImage * bgImg, const int threshold){
	if(frame == NULL || fgImg == NULL || bgImg == NULL){
		printf("Fail: There exists NULL input. Fail to get Foreground Image!\n");
		return;
	}

	if(frame->nChannels != 1 || fgImg->nChannels != 1 || bgImg->nChannels != 1){
		printf("Fail: All input image should be grayscale!\nframe channel:%d\nfgImg channel:%d\nbgImg channel:%d\n",\
			frame->nChannels, fgImg->nChannels, bgImg->nChannels);
		return;
	}

	CvMat * frameMat = cvCreateMat(frame->height, frame->width, CV_32FC1);
	CvMat * fgMat	 = cvCreateMat(fgImg->height, fgImg->width, CV_32FC1);
	CvMat * bgMat	 = cvCreateMat(bgImg->height, bgImg->width, CV_32FC1);

	cvConvert(frame, frameMat);
	cvConvert(fgImg, fgMat);
	cvConvert(bgImg, bgMat);

	cvSmooth(frameMat, frameMat, CV_GAUSSIAN, 5, 5, 4, 4); //��˹�˲��ȣ���ƽ��ͼ��

	cvAbsDiff(frameMat, bgMat, fgMat); //��ǰ֡������ͼ���(�󱳾��ȡ����ֵ)

	cvThreshold(fgMat, fgImg, threshold, 255, CV_THRESH_BINARY); //��ֵ��ǰ��ͼ(��������ض���ֵ���ж�ֵ��)

	//������̬ѧ�˲���ȥ������
	cvErode(fgImg, fgImg, 0, 1);
	cvDilate(fgImg, fgImg, 0, 1);

	//�ͷž����ڴ�
	cvReleaseMat(&frameMat);
	cvReleaseMat(&fgMat);
	cvReleaseMat(&bgMat);
}


/**
 *�ж��������Ƿ��ص�
 */
bool isOverlap(const CvRect & a, const CvRect & b){
	const CvRect * l_rect = &a,
				 * r_rect = &b;
	if(a.x > b.x){
		const CvRect * tmp = l_rect;
		l_rect = r_rect;
		r_rect = tmp;
	}

	if(l_rect->width < r_rect->x - l_rect->x)
		return false;
	else if(l_rect->y <= r_rect->y && l_rect->height >= r_rect->y - l_rect->y)
		return true;
	else if(l_rect->y > r_rect->y && r_rect->height >= l_rect->y - r_rect->y)
		return true;
	else
		return false;
}


/**
 *ǰ��λ�þ��ο�ϲ� - ���ٵ�Ԫ��
 */
void mergeRects(list<CvRect> & rects){
	int x = 0, y = 0, width = 0, height = 0;//��ʱ����
	for(list<CvRect>::iterator i = rects.begin(); i != rects.end(); ){
		bool merged = false;//������һ�������ж�i�Ƿ�merge�ǳ����ã�
		list<CvRect>::iterator j = i;
		for(j++; j != rects.end(); j++){
			if(isOverlap(*i, *j)){
				if(i->x < j->x){
					x = i->x;
					width = max(j->x - i->x + j->width, i->width);
				}
				else{
					x = j->x;
					width = max(i->x - j->x + i->width, j->width);
				}
				
				if(i->y < j->y){
					y = i->y;
					height = max(j->y - i->y + j->height, i->height);
				}
				else{
					y = j->y;
					height = max(i->y - j->y + i->height, j->height);
				}

				//�ϲ�
				j->x	  = x;
				j->y	  = y;
				j->width  = width;
				j->height = height;

				i = rects.erase(i);//ɾ�����ϲ��ע�⣺ɾ��ǰ�ߣ�i�����º��ߣ�j����
				merged = true;
			}
		}
		if(!merged)
			i++;
	}
}


/**
 *�ж��Ƿ�����ͬһ����Ŀ��
 */
bool isSameObj(const CvRect & a, const CvRect & b, const float category_factor){
	const CvRect * l_rect = &a,
				 * r_rect = &b;
	if(a.x > b.x){
		const CvRect * tmp = l_rect;
		l_rect = r_rect;
		r_rect = tmp;
	}

	int area = 0;//��¼�ص��������
	if(l_rect->width <= r_rect->x - l_rect->x)
		return false;
	else if(l_rect->y <= r_rect->y && l_rect->height > r_rect->y - l_rect->y){
		area = (l_rect->x + l_rect->width - r_rect->x) * (l_rect->y + l_rect->height - r_rect->y);
		if(area > category_factor * a.width * a.height || area > category_factor * b.width * b.height)
			return true;
		else 
			return false;
	}
	else if(l_rect->y >= r_rect->y && r_rect->height > l_rect->y - r_rect->y){
		area = (l_rect->x + l_rect->width - r_rect->x) * (r_rect->y + r_rect->height - l_rect->y);
		if(area > category_factor * a.width * a.height || area > category_factor * b.width * b.height)
			return true;
		else 
			return false;
	}
	else
		return false;
}


/**
 *ǰ�����ٲ��������ݿ�ṹ - Kernel
 */
void buildTrackDB(const char * videoFilePath, const IplImage * bgImg, list< list<tube *> > & database,\
				const int threshold, const int min_area, const int obj_num, const float extend_factor,\
				const float category_factor, const bool save_mode){
	if(bgImg == NULL){
		printf("Fail: NULL background image!\n");
		return;
	}

	if(bgImg->nChannels != 1){
		printf("Fail: Background image should be grayscale!\n");
		return;
	}

	if(!database.empty()){
		printf("Fail: Database is not empty!\n");
		return;
	}

	if(extend_factor < 0 || extend_factor > 1 || category_factor < 0 || category_factor > 1){
		printf("Fail: Invalid use of params!\n0 <= extend_factor <= 1\n0 <= category_factor <= 1\n");
		return;
	}

	CvCapture * capture = cvCaptureFromFile(videoFilePath);
	if(!capture){
		printf("Fail: Unable to open video file!\n");
		return;
	}
	int fps			= (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FPS),
		frame_count = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT),
		frame_id	= 0;
	CvSize size		= cvSize( (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH),
							(int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT) );	
	
	IplImage * frame	  = NULL,
			 * frame_gray = cvCreateImage(size, IPL_DEPTH_8U, 1),
			 * fgImg	  = cvCreateImage(size, IPL_DEPTH_8U, 1);
	
	//���±��������������Һ���
	CvMemStorage * storage = cvCreateMemStorage();
	CvSeq * first_contour  = NULL;
	IplImage * tmp		   = cvCreateImage(size, IPL_DEPTH_8U, 1);
	int find_num		   = 0,
		extend_w		   = 0,
		extend_h		   = 0;
	list<CvRect> rects; //����ҵ���ǰ����λ��
	char time[10]; //���ʱ��
	int m = 0, s = 0; //�֡���

	printf("Build track database...\n");
	while(frame = cvQueryFrame(capture)){

		cvCvtColor(frame, frame_gray, CV_BGR2GRAY); //֡ͼ���ɫת�Ҷ�
		getFgImage(frame_gray, fgImg, bgImg, threshold); //֡���ȡǰ��ͼ��
		
		cvCopy(fgImg, tmp);
		find_num = cvFindContours(tmp, storage, &first_contour, sizeof(CvContour), CV_RETR_CCOMP); //���ǰ������
		
		/**!!!Be careful with the param below!!!**/
		if(find_num > (obj_num * 10) || first_contour == NULL || frame == NULL){
			cvClearMemStorage(storage);
			continue;
		}
		
		printf("frame:%d\tfind contour num:%d\n", frame_id, find_num);
		//contour analysis
		for(CvSeq* c = first_contour; c != NULL; c = c->h_next){
			int left = 0x7fffffff, right = 0, top = 0x7fffffff, bottom = 0;
			int contourWidth  = 0,
				contourHeight = 0;
			for(int j = 0; j < c->total; j++){ 
				CvPoint * pt = (CvPoint *)cvGetSeqElem(c, j); 
				if(pt->x < left)
					left = pt->x;
				if(pt->x > right)
					right = pt->x;
				if(pt->y < top)
					top = pt->y;
				if(pt->y >bottom)
					bottom = pt->y;
			}
			contourWidth  = right - left + 1;
			contourHeight = bottom - top + 1;
			if(contourWidth * contourHeight < min_area)
				continue;
			extend_w = extend_factor * contourWidth;
			extend_h = extend_factor * contourHeight;
			left	 = (left >= extend_w ? left - extend_w : 0);
			top		 = (top >= extend_h ? top - extend_h : 0);
			right	 = ((right + extend_w) < size.width ? right + extend_w : size.width - 1);
			bottom	 = ((bottom + extend_h) < size.height ? bottom + extend_h : size.height - 1);
			rects.push_back(cvRect(left, top, right - left + 1, bottom - top + 1));
		}

		mergeRects(rects); //merge rects��һ��ɸѡ

		for(list<CvRect>::iterator iter = rects.begin(); iter != rects.end(); iter++){
			//cvRectangle(frame, cvPoint(iter->x, iter->y), cvPoint(iter->x + iter->width, iter->y + iter->height),\
						cvScalar(0,255,0), 1);//��frame�б�ʾ����󱣴�ķ�������
			m = (frame_id / fps) / 60;
			s = (frame_id / fps) % 60;
			sprintf(time, "%02d:%02d", m, s);
			cvPutText(frame, time, cvPoint(iter->x + iter->width/3, iter->y + iter->height/3),\
						&cvFont(1, 1), cvScalar(0, 0, 255));//��frame�б��ʱ��
			cvSetImageROI(frame, *iter);
			IplImage * target = cvCreateImage(cvSize((*iter).width, (*iter).height), IPL_DEPTH_8U, 3);
			cvCopy(frame, target);

			bool added = false;//�ж�ͨ�������ѭ���Ƿ�tube���뵽database
			for(list< list<tube *> >::iterator i = database.begin(); i != database.end(); i++){
				list<tube *>::iterator j = i->end();//��� *i Ϊ����Ӧ�õ� database ĩ���ˣ�ѭ��Ӧ����ֹ
				j--;//�ҵ�list<tube *>���һ��Ԫ�أ�����Ŀ������¸�������
				if(isSameObj(*iter, (*j)->position, category_factor)){//�ж���ͬһ����Ŀ��
					i->push_back(new tube(*iter, frame_id / fps, target));
					added = true;
					break;
				}
			}
			if(!added)//ɨ������databaseδ����ƥ�������½�����Ŀ������
				database.push_back(list<tube *>(1, new tube(*iter, frame_id / fps, target)));

			cvResetImageROI(frame);
			cvReleaseImage(&target);
		}

		//The following 3 lines are for debuging, comment them when debug is done!
		cvShowImage("frame", frame);
		cvShowImage("fgImg", fgImg);
		cvWaitKey(40);

		frame_id += 1;
		rects.clear();
		cvClearMemStorage(storage);

	}
	printf("Track DB has been saved! Total frame num:%d\tValid frame:%d\n", frame_count, frame_id);

	if(save_mode){//��������˱���ģʽ����database��ͼ�񱣴��ڡ�./DB���ļ�����
		if(access("./DB", 0) == 0)//�ж��Ƿ�����ļ��С�./DB��, ������ڣ���ǿ��ɾ��������ԭ���м�¼��
			system("rd /s /q DB");
		system("md DB");//�����ļ��С�./DB"
		int i = 0, j = 0;
		char filename[50];// "./DB/obj_iii_seq_jjjj_px%d_py%d_t%d.jpg"
		for(list< list<tube *> >::iterator iter1 = database.begin(); iter1 != database.end(); iter1++, i++){
			j = 0;
			for(list<tube *>::iterator iter2 = iter1->begin(); iter2 != iter1->end(); iter2++, j++){
				sprintf(filename, "./DB/obj_%03d_seq_%04d_px%d_py%d_t%d.jpg", i, j, (*iter2)->position.x, (*iter2)->position.y, (*iter2)->t_sec);
				cvSaveImage(filename, (*iter2)->target);
				printf("File %s saved.\n", filename);
			}
		}
	}

	//�ͷ��ڴ�
	cvReleaseMemStorage(&storage);
	cvReleaseImage(&tmp);
	cvReleaseImage(&frame_gray);
}


/**
 *�ںϸ������ݿ⣬�����ƵժҪ��ͬʱ�ͷ�Database�ڴ�
 */
void mergeDB(list< list<tube *> > & database, const char * videoFilePath, const int fps, const CvSize size, const IplImage * bgImg){
	if(database.empty()){
		printf("Fail: Database is empty, cannot merge!\n");
		return;
	}

	if(videoFilePath == NULL){
		printf("Fail: NULL output file path/name!\n");
		return;
	}

	CvVideoWriter * writer = cvCreateVideoWriter(videoFilePath, CV_FOURCC('M','J','P','G'), fps, size, 1);
	if(writer == NULL){
		printf("Fail: Invalid video file path/name, cannot build a video writer!\n");
		return;
	}

	IplImage * frame = cvCreateImage(size, IPL_DEPTH_8U, 3); //���֡
	tube * tmp = NULL; //��ʱtubeָ��
	vector<CvRect> rects; //����ÿ֡������λ��
	int frame_id = 0;

	printf("Merging track database...\n");
	while(!database.empty()){

		cvCopy(bgImg, frame); //ÿ�ο�ʼ�ñ���ͼƬˢ�����֡

		for(list< list<tube *> >::iterator i = database.begin(); i != database.end();){ //������������ÿ������������
			if(i->empty()){
			    i = database.erase(i);
				continue;
			}

			bool canAdd = true;
			tmp = *(i->begin());
			
			for(size_t t = 0; t != rects.size(); t++){ //����Ƿ����Ѿ��������ص�
				if(isOverlap(rects[t], tmp->position)){
					canAdd = false;
					break;
				}
			}
			
			if(canAdd){ //������Լ�����tubeд�����֡
				cvSetImageROI(frame, tmp->position);
				cvCopy(tmp->target, frame);
				cvResetImageROI(frame);
				rects.push_back(tmp->position); //�����ͼ���λ�ñ���
				delete tmp; //�ͷ�tmpָ���tube��ͼ���ڴ�
				i->pop_front();
				i++;
			}
			else
				i++;
		}
		cvWriteFrame(writer, frame); //д֡
		rects.clear(); //���rects
		printf("frame:%d written.\n", frame_id);
		frame_id += 1;
	}
	printf("Database has been merged and written to file.\n");

	//�ͷ��ڴ�
	cvReleaseImage(&frame);
	cvReleaseVideoWriter(&writer);
}


/**
 *�ͷ�Database�ڴ溯�� - ����
 */
void freeDB(list< list<tube *> > & database){
	for(list< list<tube *> >::iterator iter1 = database.begin(); iter1 != database.end(); iter1++)
		for(list<tube *>::iterator iter2 = iter1->begin(); iter2 != iter1->end();){
			delete *iter2;
			iter2 = (*iter1).erase(iter2);
		}
}