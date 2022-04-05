
// FlatTubeDefectDetectionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "FlatTubeDefectDetection.h"
#include "FlatTubeDefectDetectionDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IMAGE_WIDTH 600
#define IMAGE_HEIGHT 22000
#define SECTION_NUMBER 8

#define SAMPLE_NUM 100
#define MINI_ERROR 5 //20
#define CURVE_LENGTH 151

struct SectionData
{
	int LocalPeakNum;
	int LocalPeakPosition[100];
};

CString PicFile = _T("");
CString m_strFolderPath;
bool Folder_Flag = 0;
int OriginalWidth;
int OriginalHeight;
int ObjectNum = 0;
int ProgressTotolNumber = 0;
int ProgressCurNumber = 0;
int OK_Num = 0, NG_Num = 0;
int Debug_mode = 1;
double alg_time_cnt = 0;

CvPoint3D32f Control_pts_draw[4];
CvPoint3D32f Control_pts[4 * SAMPLE_NUM];
float BezierFirstDerivat(float p0, float p1, float p2, float p3, float t);
float BezierSecondDerivat(float p0, float p1, float p2, float p3, float t);
float BezierCurvature(float FirstDerivat_x, float SecondDerivat_x, float FirstDerivat_y, float SecondDerivat_y);
CvPoint3D32f pointAdd(CvPoint3D32f p, CvPoint3D32f q);
CvPoint3D32f pointTimes(float c, CvPoint3D32f p);
CvPoint3D32f Bernstein(float u, CvPoint3D32f *p);

/*
The first derivative of cubic Bezier curve at point t
*/
float BezierFirstDerivat(float p0, float p1, float p2, float p3, float t)
{
	float derivate;

	derivate = 3 * ((p1 - p0)*(1 - t)*(1 - t) + (p2 - p1)*t*(1 - t) + (p3 - p2)*t*t);

	return derivate;
}

/*
The second derivative of cubic Bezier curve at point t
*/
float BezierSecondDerivat(float p0, float p1, float p2, float p3, float t)
{
	float derivate;

	derivate = 6 * ((p2 - 2 * p1 + p0)*(1 - t) + (p3 - 2 * p2 + p1)*t);

	return derivate;
}

/*
Curvature of the Bezier curve use first derivative and second derivative
*/
float BezierCurvature(float FirstDerivat_x, float SecondDerivat_x, float FirstDerivat_y, float SecondDerivat_y)
{
	float Curvature;
	Curvature = fabs(FirstDerivat_x*SecondDerivat_y - SecondDerivat_x*FirstDerivat_y) / pow(sqrt(fabs(FirstDerivat_x*FirstDerivat_x + FirstDerivat_y*FirstDerivat_y)), 3);
	return Curvature;
}

CvPoint3D32f pointAdd(CvPoint3D32f p, CvPoint3D32f q)
{
	p.x += q.x; p.y += q.y; p.z += q.z;
	return p;
}

CvPoint3D32f pointTimes(float c, CvPoint3D32f p)
{
	p.x *= c; p.y *= c; p.z *= c;
	return p;
}

CvPoint3D32f Bernstein(float u, CvPoint3D32f *p)
{
	CvPoint3D32f a, b, c, d, r;

	a = pointTimes(pow(u, 3), p[0]);
	b = pointTimes(3 * pow(u, 2)*(1 - u), p[1]);
	c = pointTimes(3 * u*pow((1 - u), 2), p[2]);
	d = pointTimes(pow((1 - u), 3), p[3]);

	r = pointAdd(pointAdd(a, b), pointAdd(c, d));

	return r;
}


// 

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV

// 
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CFlatTubeDefectDetectionDlg 



CFlatTubeDefectDetectionDlg::CFlatTubeDefectDetectionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FLATTUBEDEFECTDETECTION_DIALOG, pParent)
	, m_StrEditFilePath(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFlatTubeDefectDetectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILE_PATH, m_StrEditFilePath);
}

BEGIN_MESSAGE_MAP(CFlatTubeDefectDetectionDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_OPEN_FILE, &CFlatTubeDefectDetectionDlg::OnBnClickedBtnOpenFile)
	ON_BN_CLICKED(IDC_BTN_PROCESS_SILK, &CFlatTubeDefectDetectionDlg::OnBnClickedBtnProcessSilk)
END_MESSAGE_MAP()


// CFlatTubeDefectDetectionDlg 

BOOL CFlatTubeDefectDetectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 

	// IDM_ABOUTBOX 
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 
	//  
	SetIcon(m_hIcon, TRUE);			// 
	SetIcon(m_hIcon, FALSE);		// 

	// TODO: 

	return TRUE;  // 
}

void CFlatTubeDefectDetectionDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFlatTubeDefectDetectionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFlatTubeDefectDetectionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CFlatTubeDefectDetectionDlg::OnBnClickedBtnOpenFile()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strFile = _T("");
	CString strModelName = _T("");
	CFileDialog   dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Describe Files (*.bmp)|*.bmp|All Files (*.*)|*.*||"), NULL);
	if (dlgFile.DoModal())
	{
		//UpdateData(FALSE);
		PicFile = dlgFile.GetPathName();
		m_StrEditFilePath = PicFile;
		Folder_Flag = 0;
		UpdateData(FALSE);
	}
	if (PicFile.IsEmpty())
	{
		return;
	}
}

void CFlatTubeDefectDetectionDlg::DrawMat(cv::Mat& img, UINT nID)
{

	CRect rect;
	Mat imgSrcPreview = Mat::zeros(img.size(), CV_8UC3);
	GetDlgItem(nID)->GetClientRect(&rect);  // 
	cv::resize(imgSrcPreview, imgSrcPreview, cv::Size(rect.Width(), rect.Height()));// 

	double resize_ratio, resize_ratio_x, resize_ratio_y;

	Mat MaskModel;
	int temp_cols;
	int temp_rows;
	Rect roi_Rect_preview;

	if (img.rows >= img.cols)
	{
		resize_ratio = (double)(img.rows) / (double)(rect.Height());
		resize_ratio_x = (double)(img.cols) / (double)(rect.Width());
		resize_ratio_y = (double)(img.rows) / (double)(rect.Height());
		temp_cols = (rect.Width() - (double)(img.cols) / resize_ratio) / 2;
		roi_Rect_preview = Rect(temp_cols, 0, (img.cols) / resize_ratio, (img.rows) / resize_ratio);

		MaskModel = img.clone();
		cv::resize(MaskModel, MaskModel, cv::Size((img.cols) / resize_ratio, (img.rows) / resize_ratio));
		MaskModel.copyTo(imgSrcPreview(roi_Rect_preview));
	}
	else
	{
		resize_ratio = (double)(img.cols) / (double)(rect.Width());
		temp_rows = (rect.Height() - (double)(img.rows) / resize_ratio) / 2;
		roi_Rect_preview = Rect(0, temp_rows, (img.cols) / resize_ratio, (img.rows) / resize_ratio);

		MaskModel = img.clone();
		cv::resize(MaskModel, MaskModel, cv::Size((img.cols) / resize_ratio, (img.rows) / resize_ratio));
		MaskModel.copyTo(imgSrcPreview(roi_Rect_preview));
	}

	cv::Mat imgTmp;
	imgTmp = imgSrcPreview.clone();
	
	GetDlgItem(nID)->GetClientRect(&rect);  
	cv::resize(imgTmp, imgTmp, cv::Size(rect.Width(), rect.Height()));

																   
	switch (imgTmp.channels())
	{
	case 1:
		cv::cvtColor(imgTmp, imgTmp, CV_GRAY2BGRA); 
		break;
	case 3:
		cv::cvtColor(imgTmp, imgTmp, CV_BGR2BGRA);  
		break;
	default:
		break;
	}

	int pixelBytes = imgTmp.channels()*(imgTmp.depth() + 1); 
															 
	BITMAPINFO bitInfo;
	bitInfo.bmiHeader.biBitCount = 8 * pixelBytes;
	bitInfo.bmiHeader.biWidth = imgTmp.cols;
	bitInfo.bmiHeader.biHeight = -imgTmp.rows;
	bitInfo.bmiHeader.biPlanes = 1;
	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo.bmiHeader.biCompression = BI_RGB;
	bitInfo.bmiHeader.biClrImportant = 0;
	bitInfo.bmiHeader.biClrUsed = 0;
	bitInfo.bmiHeader.biSizeImage = 0;
	bitInfo.bmiHeader.biXPelsPerMeter = 0;
	bitInfo.bmiHeader.biYPelsPerMeter = 0;
	
	CDC *pDC = GetDlgItem(nID)->GetDC();
	::StretchDIBits(
		pDC->GetSafeHdc(),
		0, 0, rect.Width(), rect.Height(),
		0, 0, rect.Width(), rect.Height(),
		imgTmp.data,
		&bitInfo,
		DIB_RGB_COLORS,
		SRCCOPY
		);
	ReleaseDC(pDC);

}

void CFlatTubeDefectDetectionDlg::OnBnClickedBtnProcessSilk()
{
	// TODO: 在此添加控件通知处理程序代码
	//std::string strStl;
	USES_CONVERSION;
	std::string strStl = W2A(PicFile);
	Mat imgSrc, imgSrc_gray, img_result;
	imgSrc = imread(strStl);
	PARAMETERS para;
	para.Flag = 0;
	para.Threshold1 = 100;
	para.Threshold2 = 100;

	DrawMat(imgSrc, IDC_STATIC_IMAGE_INPUT);
	img_result = ImageProcess(imgSrc, para);
	
	AfxMessageBox(_T("Done!"));
}

Mat CFlatTubeDefectDetectionDlg::ImageProcess(cv::Mat& img, PARAMETERS para)
{
	Mat result;
	Mat imgSrc_gray;
	Mat imgSrc;
	//cvtColor(img, imgSrc_gray, CV_BGR2GRAY);
	//threshold(imgSrc_gray, result, 100, 255, CV_THRESH_BINARY);

	FILE *rfile;
	char* data_file = "./result/local_hist.xls";
	rfile = fopen(data_file, "wt");
	if (!rfile)
	{
		AfxMessageBox(_T("ERROR: log file can not created  \n"));
		return result;
	}
	//fprintf(rfile,"cur \n");
	fprintf(rfile, "序号 \t 图片 \t 结果\t 丝槽 \t 其他缺陷 \t 大小  \n");

	FILE *rfile1;
	char* data_file1 = "./result/曲率.xls";
	rfile1 = fopen(data_file1, "wt");
	if (!rfile1)
	{
		AfxMessageBox(_T("ERROR: log file1 can not created  \n"));
		return result;
	}

	FILE *rfile2;
	char* data_file2 = "./result/累计曲率.xls";
	rfile2 = fopen(data_file2, "wt");
	if (!rfile2)
	{
		AfxMessageBox(_T("ERROR: log file2 can not created  \n"));
		return result;
	}
	std::string strStl;
	CFileFind finder;
	CStringList filelist;//文件列表
	CStringList filename_list;//文件列表
	unsigned long filenum = 0;

	int ng_flag = 0, inter_ng_flag = 0;
	Mat imgSrc1, imgSrc2, imgSrc1_gray, imgSrc2_gray;
	Mat gray_down_2, gray_down_4;

	
	
	filenum = 1;
	for (int image_cnt = 0; image_cnt<filenum; image_cnt++)
	{
		MSG	msg;
		if (PeekMessage(&msg, (HWND)NULL, 0, 0, PM_REMOVE))
		{
			::SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
		}

		POSITION position;
		POSITION position_name;
		CString temp;
		std::string file_name;
		
		//imgSrc = imread(strStl); //PicFile  "test1.jpg"
		imgSrc = img.clone();
		DWORD nowtime = 0, lastime = 0;
		lastime = GetTickCount();

		imgSrc1 = Mat::zeros(imgSrc.size(), CV_8UC3);
		imgSrc2 = Mat::zeros(imgSrc.size(), CV_8UC3);
		imgSrc1_gray = Mat::zeros(imgSrc.size(), CV_8UC3);
		imgSrc2_gray = Mat::zeros(imgSrc.size(), CV_8UC3);

		imgSrc1 = imgSrc.clone();
		imgSrc2 = imgSrc.clone();

		cvtColor(imgSrc1, imgSrc1_gray, CV_BGR2GRAY);
		cvtColor(imgSrc1, imgSrc2_gray, CV_BGR2GRAY);

		//判断扁管是否在图像的边缘，如果是，则代表扁管采集不完整；
		//只统计第四行，第四列，倒数第四行，倒数第四列 是否存在一定数量的两点
		int total_side_number = 0;
		uchar *pData = imgSrc1_gray.ptr<uchar>(3); //第四行
		uchar *pData2 = imgSrc1_gray.ptr<uchar>(imgSrc1_gray.rows - 3); //倒数第四行
		for (int i = 0; i<imgSrc1_gray.cols; i++)
		{
			uchar val1 = *(pData + i);
			uchar val2 = *(pData2 + i);
			if ((val1 > 100) || (val2 > 100))
			{
				total_side_number++;
			}
		}
		for (int i = 0; i<imgSrc1_gray.rows; i++)
		{
			uchar *pData = imgSrc1_gray.ptr<uchar>(i);
			uchar val1 = *(pData + 3);  //第四列
			uchar val2 = *(pData + imgSrc1_gray.cols - 3); //倒数第四列
			if ((val1 > 100) || (val2 > 100))
			{
				total_side_number++;
			}
		}

		if (total_side_number > 100)
		{
			if (Folder_Flag != 1)
				AfxMessageBox(_T("图像采集不完整！"));
			return result;
		}
		Mat coutours;

		int num_cnt = 0;
		int mean_val = 0;

		//float pro_hist[IMAGE_HEIGHT];
		//float pro_hist2[IMAGE_WIDTH]; 
		int mean_hist = 0, mean_hist_cnt = 0;
		int mean_hist2 = 0, mean_hist_cnt2 = 0;

		/////////////////////////////////////////////////////////////
		//20200403
		Mat GetROI_img, GetROI_img_color;
		GetROI_img = Mat::zeros(imgSrc.size(), CV_8UC1);
		GetROI_img_color = Mat::zeros(imgSrc.size(), CV_8UC3);
		threshold(imgSrc1_gray, GetROI_img, 50, 255, CV_THRESH_BINARY);
		if (Debug_mode == 1)
			imwrite("./result/GetROI_img.jpg", GetROI_img);
		//轮廓检测
		std::vector<std::vector<Point>> contours_temp;
		std::vector<Vec4i> hierarchy_temp;
		double contour_area_temp = 0, max_area_temp = 0;
		int max_index_temp = 0;
		findContours(GetROI_img, contours_temp, hierarchy_temp, CV_RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));
		for (int i = 0; i < contours_temp.size(); i++)
		{
			contour_area_temp = contours_temp[i].size();
			if (contour_area_temp > max_area_temp)
			{
				max_area_temp = contour_area_temp;
				max_index_temp = i;
			}
		}
		RotatedRect rectPoint = minAreaRect((Mat)contours_temp[max_index_temp]);
		Point2f vertices[4];
		Point2f vertices_temp[4];
		float mini_y = 50000;
		int mini_y_index = 0, mini2_y_index = 0;
		//获取旋转矩形的四个顶点
		rectPoint.points(vertices);
		for (int temp_cnt = 0; temp_cnt<4; temp_cnt++)
		{
			if (vertices[temp_cnt].x < 0)
			{
				vertices[temp_cnt].x = 0;
			}
			if (vertices[temp_cnt].y < 0)
			{
				vertices[temp_cnt].y = 0;
			}
		}

		float RotateAngle = rectPoint.angle;

		if (RotateAngle < -20)
		{
			RotateAngle = 90 + RotateAngle;
		}

		cv::Point2f center(imgSrc1.cols / 2, imgSrc1.rows / 2);
		cv::Mat rot = cv::getRotationMatrix2D(center, RotateAngle, 1);
		cv::Rect bbox = cv::RotatedRect(center, imgSrc1.size(), RotateAngle).boundingRect();

		bbox.width = imgSrc1.cols;
		bbox.height = imgSrc1.rows;

		rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
		rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;
		cv::warpAffine(imgSrc, imgSrc, rot, bbox.size());
		if (Debug_mode == 1)
			imwrite("./result/Rotated.jpg", GetROI_img_color);
		Mat ImageRotatedROI;
		int ROI_width = 0, ROI_height = 0;
		ROI_width = rectPoint.size.width - 20;
		ROI_height = rectPoint.size.height - 20;
		int temp_data0 = ROI_width % 4;
		ROI_width = ROI_width - temp_data0;
		temp_data0 = ROI_height % 4;
		ROI_height = ROI_height - temp_data0;
		Rect ROI_temp0;
		if (ROI_width <= ROI_height)
		{
			ROI_temp0.x = rectPoint.center.x - ROI_width / 2;
			ROI_temp0.y = rectPoint.center.y - ROI_height / 2;
			ROI_temp0.width = ROI_width;
			ROI_temp0.height = ROI_height;

			if (ROI_temp0.x < 0)
			{
				ROI_temp0.x = 0;
			}
			if (ROI_temp0.y < 0)
			{
				ROI_temp0.y = 0;
			}
		}
		else
		{
			ROI_temp0.x = rectPoint.center.x - ROI_height / 2;
			ROI_temp0.y = rectPoint.center.y - ROI_width / 2;
			ROI_temp0.width = ROI_height;
			ROI_temp0.height = ROI_width;
			if (ROI_temp0.x < 0)
			{
				ROI_temp0.x = 0;
			}
			if (ROI_temp0.y < 0)
			{
				ROI_temp0.y = 0;
			}
		}
		Mat ResultROI;
		ResultROI = imgSrc(ROI_temp0);
		cvtColor(ResultROI, ResultROI, CV_BGR2GRAY);

		//ImageRotatedROI = imgSrc(Rect(rectPoint.center.x - ROI_width/2,rectPoint.center.y - ROI_height/2,ROI_width,ROI_height));
		if (Debug_mode == 1)
			imwrite("./result/ImageRotatedROI.jpg", ResultROI);

		//另一个线程开始处理
		//Thread1Image = ResultROI.clone();
		//ImageLoaded_Flag = 1;
		if (Debug_mode == 1)
		{
			imwrite("./result/ResultROI.jpg", ResultROI);
		}


		/*DWORD nowtime=0,lastime=0;
		lastime=GetTickCount();*/
		//调试显示用

		Mat reverseImage;
		reverseImage = 255 - ResultROI;//ImageRotatedROI; //matResult
		
		if (Debug_mode == 1)
			imwrite("./result/反色.jpg", reverseImage);

		Mat MyFilterImage;
		Mat MyFilterImage2;
		Mat conv_kernel = (Mat_<char>(3, 3) <<
			0, -1, 0,
			-1, 5, -1,
			0, -1, 0);
		Mat conv_kernel5 = (Mat_<char>(5, 5) <<
			0, 0, -1, 0, 0,
			0, -1, 3, -1, 0,
			-1, 3, 5, 3, -1,
			0, -1, 3, -1, 0,
			0, 0, -1, 0, 0);
		Mat conv_kernel2 = (Mat_<char>(7, 7) <<
			-3, -3, -3, -3, -3, -3, -3,
			-3, -1, -1, -1, -1, -1, -3,
			-3, -1, 5, 5, 5, -1, -3,
			-3, -1, 5, 10, 5, -1, -3,
			-3, -1, 5, 5, 5, -1, -3,
			-3, -1, -1, -1, -1, -1, -3,
			-3, -3, -3, -3, -3, -3, -3);


		//滤波
		//boxFilter(reverseImage, reverseImage, -1, Size(5, 5));
		if (Debug_mode == 1)
			imwrite("./result/反色-滤波.jpg", reverseImage);



		//gray_down_4 = reverseImage.clone();
		int new_width, new_height, temp_data;
		new_width = reverseImage.cols / 4;
		new_height = reverseImage.rows / 4;
		temp_data = new_width % 4;
		new_width = new_width - temp_data;
		resize(reverseImage, gray_down_4, Size(new_width, new_height));

		Rect ROI_temp;
		ROI_temp.x = gray_down_4.cols / 12; //4
		ROI_temp.y = gray_down_4.rows / 100; //4
		ROI_temp.width = gray_down_4.cols - gray_down_4.cols / 6; //8
		ROI_temp.height = gray_down_4.rows - gray_down_4.rows / 50; //8
		new_width = new_width - gray_down_4.cols / 6;//8
		new_height = new_height - gray_down_4.rows / 50;//8
		gray_down_4 = gray_down_4(ROI_temp);
		Mat mean;
		Mat stdDev;
		meanStdDev(gray_down_4, mean, stdDev);
		double avg, stddev;
		mean_val = mean.ptr<double>(0)[0];
		stddev = stdDev.ptr<double>(0)[0];


		int adaptive_val;
		adaptive_val = 1.3*mean_val; //1.2
		if (adaptive_val > 200)
		{
			adaptive_val = 200;
		}
		threshold(gray_down_4, gray_down_4, 0, 255, CV_THRESH_OTSU);
		//threshold(gray_down_4, gray_down_4, adaptive_val, 255, CV_THRESH_BINARY);
		if (Debug_mode == 1)
			imwrite("./result/降采样自适应阈值分割结果.jpg", gray_down_4);
		Mat gray_down_copy;
		gray_down_copy = gray_down_4.clone();
		cvtColor(gray_down_copy, gray_down_copy, CV_GRAY2BGR);

		filter2D(gray_down_4, MyFilterImage, -1, conv_kernel);
		filter2D(gray_down_4, MyFilterImage2, CV_32F, conv_kernel2);
		convertScaleAbs(MyFilterImage2, MyFilterImage2);

		if (Debug_mode == 1)
		{
			imwrite("./result/新滤波器.jpg", MyFilterImage);
			imwrite("./result/新滤波器2.jpg", MyFilterImage2);
		}

		SectionData SectData[10];
		int local_hist_height;
		local_hist_height = new_height / 10;
		int local_hist_data[10][500];
		int local_hist_length = gray_down_4.cols - 10;
		for (int n = 0; n<10; n++)
		{
			for (int m = 0; m<500; m++)
			{
				local_hist_data[n][m] = 0;
			}
		}
		for (int kk = 0; kk<10; kk++)
		{
			mean_hist_cnt = 0;
			mean_hist = 0;
			CArray<int, int>ProHist;
			ProHist.SetSize(new_width, -1);
			for (int i = 5; i<gray_down_4.cols - 5; i++)
			{
				int sum = 0;
				for (int j = local_hist_height*kk; j<local_hist_height*(kk + 1); j++)
				{
					uchar val;
					val = gray_down_4.at<uchar>(j, i);
					if (val > 0)
					{
						sum++;
						ProHist.SetAt(i, sum);
					}
				}
				int data = 0;
				data = ProHist.GetAt(i);

				//记录数据供分析用
				local_hist_data[kk][i - 5] = data;

				if (data > 1)
					mean_hist_cnt++;
				mean_hist += data;
			}
		}


		//画出投影直方图曲线
		Mat HistImage;
		Mat HistImageSection;
		Point CurvePoint[10][10000];
		int CurvePointCnt[10];
		int index = 0;
		for (int k = 0; k<10; k++)
		{
			CurvePointCnt[k] = 0;
		}
		HistImage = Mat::zeros(cvSize((gray_down_4.cols + 10) * 5, gray_down_4.rows), CV_8UC1); // 全部10段
		HistImageSection = Mat::zeros(cvSize((gray_down_4.cols + 10) * 5, gray_down_4.rows/20+5), CV_8UC1); // /10取其中一段测试，本应gray_down_4.rows/10，为了显示效果更好改成这样
		for (int n = 0; n<10; n++)
		{
			index = 0;
			for (int m = 0; m<local_hist_length - 1; m++)
			{
				//其中一段
				if(n == SECTION_NUMBER)
					line(HistImageSection,Point(5*(m+5),local_hist_height-local_hist_data[n][m]),Point(5*(m+5+1),local_hist_height-local_hist_data[n][m+1]),Scalar(255,255,255),1,8,0);
				//全部
				line(HistImage, Point(5 * (m + 5), local_hist_height*n + local_hist_height - local_hist_data[n][m]), Point(5 * (m + 5 + 1), local_hist_height*n + local_hist_height - local_hist_data[n][m + 1]), Scalar(255, 255, 255), 1, 8, 0);
				//LineIterator it(HistImage, Point(5*(m+5),local_hist_height-local_hist_data[n][m]),Point(5*(m+5+1),local_hist_height-local_hist_data[n][m+1]),8);
				LineIterator it(HistImage, Point(5 * (m + 5), local_hist_height*n + local_hist_height - local_hist_data[n][m]), Point(5 * (m + 5 + 1), local_hist_height*n + local_hist_height - local_hist_data[n][m + 1]), 8);
				int start = 0;
				if (m > 0)
					start = 1;
				for (int i = start; i < it.count; i++, ++it)
				{
					cv::Point pt(it.pos());
					CurvePoint[n][index + i - start] = pt;
					CurvePointCnt[n]++;
				}
				index += it.count - 1;
			}
		}
		if (Debug_mode == 1)
		{
			imwrite("./result/HistImage.jpg", HistImage);
			imwrite("./result/HistImageSection.jpg", HistImageSection);
		}


		Mat HistImageTest;
		Mat HistImageSectionTest;
		HistImageTest = HistImage.clone();
		HistImageSectionTest = HistImageSection.clone();
		cvtColor(HistImageTest, HistImageTest, CV_GRAY2BGR);
		cvtColor(HistImageSectionTest, HistImageSectionTest, CV_GRAY2BGR);

#if 1
		for (int kkk = 0; kkk<10; kkk++)
		{
			SectData[kkk].LocalPeakNum = 0;
		}
		int HistSectCount = 0;
		for (HistSectCount = 0; HistSectCount<10; HistSectCount++)
		{
			int FrameW = HistImage.cols;
			int FrameH = HistImage.rows;
			double* curvative = new double[CurvePointCnt[HistSectCount]];
			float*  accumulate = new float[CurvePointCnt[HistSectCount]];
			CvPoint3D32f* optimal_control_pts = new CvPoint3D32f[4 * CurvePointCnt[HistSectCount]];

			int data_record_cnt=0, data_record_cnt_pre=0;

			double curvative_max = 0, curvative_temp = 0;
			float mean_curvative = 0;
			int total = CurvePointCnt[HistSectCount];
			//int curve_length=21;
			//曲率分析
			for (int i = CURVE_LENGTH / 2; i<CurvePointCnt[HistSectCount] - CURVE_LENGTH / 2;) //i++
			{
				curvative[i] = 0;
				CvPoint sample_pt[CURVE_LENGTH];
				int left = HistImage.cols - 5, right = 0, top = HistImage.rows - 5, bottom = 0;
				for (int ii = 0; ii<CURVE_LENGTH; ii++)
				{
					if ((i <= (CurvePointCnt[HistSectCount] - 1 - CURVE_LENGTH / 2)) && (i >= CURVE_LENGTH / 2))
					{
						sample_pt[ii].x = CurvePoint[HistSectCount][i - CURVE_LENGTH / 2 + ii].x;
						sample_pt[ii].y = CurvePoint[HistSectCount][i - CURVE_LENGTH / 2 + ii].y;
					}
				}
				for (int ii = 0; ii<CURVE_LENGTH; ii++)
				{
					if (sample_pt[ii].x < left)
					{
						left = sample_pt[ii].x;
					}
					if (sample_pt[ii].x > right)
					{
						right = sample_pt[ii].x;
					}
					if (sample_pt[ii].y < top)
					{
						top = sample_pt[ii].y;
					}
					if (sample_pt[ii].y > bottom)
					{
						bottom = sample_pt[ii].y;
					}
				}

				float error_square[SAMPLE_NUM], error_square_min = 1000000;
				float confidence[SAMPLE_NUM], confidence_sum = 0, confidence_max = 0;
				int error_min_index = 0;

				CvMat* noise_x_pt0 = cvCreateMat(SAMPLE_NUM, 1, CV_32FC1);
				CvMat* noise_y_pt0 = cvCreateMat(SAMPLE_NUM, 1, CV_32FC1);
				CvMat* noise_x_pt1 = cvCreateMat(SAMPLE_NUM, 1, CV_32FC1);
				CvMat* noise_y_pt1 = cvCreateMat(SAMPLE_NUM, 1, CV_32FC1);
				CvMat* noise_x_pt2 = cvCreateMat(SAMPLE_NUM, 1, CV_32FC1);
				CvMat* noise_y_pt2 = cvCreateMat(SAMPLE_NUM, 1, CV_32FC1);
				CvMat* noise_x_pt3 = cvCreateMat(SAMPLE_NUM, 1, CV_32FC1);
				CvMat* noise_y_pt3 = cvCreateMat(SAMPLE_NUM, 1, CV_32FC1);

				bool find_min_flag = 0;
				int region_threshold = 15; //15
				unsigned char mini_done_flag = 0;

				//while(!mini_done_flag)
				{
					double temp_left, temp_right;
					if (left >= region_threshold)
					{
						temp_left = left - region_threshold;
					}
					else
						temp_left = 0;  //left
					if (right < (FrameW - region_threshold))
					{
						temp_right = right + region_threshold;
					}
					else
						temp_right = FrameW; //right
					CvRNG rng_x_pt1(cvGetTickCount());
					cvRandInt(&rng_x_pt1);
					cvRandArr(&rng_x_pt1, noise_x_pt1, CV_RAND_UNI, cvScalarAll(temp_left), cvScalarAll(temp_right));
					CvRNG rng_x_pt2(cvGetTickCount());
					cvRandInt(&rng_x_pt2);
					cvRandArr(&rng_x_pt2, noise_x_pt2, CV_RAND_UNI, cvScalarAll(temp_left), cvScalarAll(temp_right));

					double temp_top, temp_bottom;
					if (top >= 4 * region_threshold)
					{
						temp_top = top - 4 * region_threshold;
					}
					else
						temp_top = 0; //top
					if (bottom < (FrameH - 4 * region_threshold))
					{
						temp_bottom = bottom + 4 * region_threshold;
					}
					else
						temp_bottom = FrameH; //bottom
					CvRNG rng_y_pt1(cvGetTickCount());
					cvRandInt(&rng_y_pt1);
					cvRandArr(&rng_y_pt1, noise_y_pt1, CV_RAND_UNI, cvScalarAll(temp_top), cvScalarAll(temp_bottom));
					CvRNG rng_y_pt2(cvGetTickCount());
					cvRandInt(&rng_y_pt2);
					cvRandArr(&rng_y_pt2, noise_y_pt2, CV_RAND_UNI, cvScalarAll(temp_top), cvScalarAll(temp_bottom));

					//第一个点x
					int end_point_threshold = 3;
					temp_left = sample_pt[0].x;
					temp_right = sample_pt[0].x;
					if (temp_left >= end_point_threshold)
					{
						temp_left = temp_left - end_point_threshold;
					}
					else
						temp_left = temp_left;
					if (temp_right < (FrameW - end_point_threshold))
					{
						temp_right = temp_right + end_point_threshold;
					}
					else
						temp_right = temp_right;
					CvRNG rng_x_pt0(cvGetTickCount());
					cvRandInt(&rng_x_pt0);
					cvRandArr(&rng_x_pt0, noise_x_pt0, CV_RAND_UNI, cvScalarAll(temp_left), cvScalarAll(temp_right));
					//第四个点x
					//int end_point_threshold=3;
					temp_left = sample_pt[CURVE_LENGTH - 1].x;
					temp_right = sample_pt[CURVE_LENGTH - 1].x;
					if (temp_left >= end_point_threshold)
					{
						temp_left = temp_left - end_point_threshold;
					}
					else
						temp_left = temp_left;
					if (temp_right < (FrameW - end_point_threshold))
					{
						temp_right = temp_right + end_point_threshold;
					}
					else
						temp_right = temp_right;
					CvRNG rng_x_pt3(cvGetTickCount());
					cvRandInt(&rng_x_pt3);
					cvRandArr(&rng_x_pt3, noise_x_pt3, CV_RAND_UNI, cvScalarAll(temp_left), cvScalarAll(temp_right));

					//第一个点y
					temp_top = temp_bottom = sample_pt[0].y;
					if (temp_top >= end_point_threshold)
					{
						temp_top = temp_top - end_point_threshold;
					}
					else
						temp_top = temp_top;
					if (temp_bottom < (FrameH - end_point_threshold))
					{
						temp_bottom = temp_bottom + end_point_threshold;
					}
					else
						temp_bottom = temp_bottom;
					CvRNG rng_y_pt0(cvGetTickCount());
					cvRandInt(&rng_y_pt0);
					cvRandArr(&rng_y_pt0, noise_y_pt0, CV_RAND_UNI, cvScalarAll(temp_bottom), cvScalarAll(temp_top));

					//第四个点y
					temp_top = temp_bottom = sample_pt[CURVE_LENGTH - 1].y;
					if (temp_top >= end_point_threshold)
					{
						temp_top = temp_top - end_point_threshold;
					}
					else
						temp_top = temp_top;
					if (temp_bottom < (FrameH - end_point_threshold))
					{
						temp_bottom = temp_bottom + end_point_threshold;
					}
					else
						temp_bottom = temp_bottom;
					CvRNG rng_y_pt3(cvGetTickCount());
					cvRandInt(&rng_y_pt3);
					cvRandArr(&rng_y_pt3, noise_y_pt3, CV_RAND_UNI, cvScalarAll(temp_bottom), cvScalarAll(temp_top));

					//while(!mini_done_flag)
					//error_square_min=1000000;
					for (int j = 0; j<SAMPLE_NUM; j++)
					{
						error_square[j] = 0;

						Control_pts[j * 4 + 0].x = noise_x_pt0->data.fl[j];
						Control_pts[j * 4 + 0].y = noise_y_pt0->data.fl[j];
						Control_pts[j * 4 + 1].x = noise_x_pt1->data.fl[j];
						Control_pts[j * 4 + 1].y = noise_y_pt1->data.fl[j];
						Control_pts[j * 4 + 2].x = noise_x_pt2->data.fl[j];
						Control_pts[j * 4 + 2].y = noise_y_pt2->data.fl[j];
						Control_pts[j * 4 + 3].x = noise_x_pt3->data.fl[j];
						Control_pts[j * 4 + 3].y = noise_y_pt3->data.fl[j];

						if (Control_pts[j * 4 + 0].x <0)
						{
							Control_pts[j * 4 + 0].x = 0;
						}
						else if (Control_pts[j * 4 + 0].x > FrameW)
						{
							Control_pts[j * 4 + 0].x = FrameW;
						}
						if (Control_pts[j * 4 + 0].y <0)
						{
							Control_pts[j * 4 + 0].y = 0;
						}
						else if (Control_pts[j * 4 + 0].y > FrameH)
						{
							Control_pts[j * 4 + 0].y = FrameH;
						}
						if (Control_pts[j * 4 + 1].x <0)
						{
							Control_pts[j * 4 + 1].x = 0;
						}
						else if (Control_pts[j * 4 + 1].x > FrameW)
						{
							Control_pts[j * 4 + 1].x = FrameW;
						}
						if (Control_pts[j * 4 + 1].y <0)
						{
							Control_pts[j * 4 + 1].y = 0;
						}
						else if (Control_pts[j * 4 + 1].y > FrameH)
						{
							Control_pts[j * 4 + 1].y = FrameH;
						}
						if (Control_pts[j * 4 + 2].x <0)
						{
							Control_pts[j * 4 + 2].x = 0;
						}
						else if (Control_pts[j * 4 + 2].x > FrameW)
						{
							Control_pts[j * 4 + 2].x = FrameW;
						}
						if (Control_pts[j * 4 + 2].y <0)
						{
							Control_pts[j * 4 + 2].y = 0;
						}
						else if (Control_pts[j * 4 + 2].y > FrameH)
						{
							Control_pts[j * 4 + 2].y = FrameH;
						}
						if (Control_pts[j * 4 + 3].x <0)
						{
							Control_pts[j * 4 + 3].x = 0;
						}
						else if (Control_pts[j * 4 + 3].x > FrameW)
						{
							Control_pts[j * 4 + 3].x = FrameW;
						}
						if (Control_pts[j * 4 + 3].y <0)
						{
							Control_pts[j * 4 + 3].y = 0;
						}
						else if (Control_pts[j * 4 + 3].y > FrameH)
						{
							Control_pts[j * 4 + 3].y = FrameH;
						}

						//21个数据测试点
						for (int m = 0; m<CURVE_LENGTH; m++)
						{
							float curve_x, curve_y, t;
							t = float(m) / (float)CURVE_LENGTH;
							curve_x = pow((1 - t), 3)*Control_pts[j * 4 + 0].x + 3 * t*(1 - t)*(1 - t)*Control_pts[j * 4 + 1].x + 3 * t*t*(1 - t)*Control_pts[j * 4 + 2].x + pow(t, 3)*Control_pts[j * 4 + 3].x;
							curve_y = pow((1 - t), 3)*Control_pts[j * 4 + 0].y + 3 * t*(1 - t)*(1 - t)*Control_pts[j * 4 + 1].y + 3 * t*t*(1 - t)*Control_pts[j * 4 + 2].y + pow(t, 3)*Control_pts[j * 4 + 3].y;
							error_square[j] += (fabs(float(sample_pt[m].x)) - fabs(float(curve_x)))*(fabs(float(sample_pt[m].x)) - fabs(float(curve_x))) + (fabs(float(sample_pt[m].y)) - fabs(float(curve_y)))*(fabs(float(sample_pt[m].y)) - fabs(float(curve_y)));

						}
						if (error_square[j] < error_square_min)
						{
							error_square_min = error_square[j];
							error_min_index = j;
						}
					}
					if (error_square_min < MINI_ERROR)
					{
						mini_done_flag = 1;
					}
					else
					{
						mini_done_flag = 0;
					}
				}

				float first_order_x, first_order_y, second_order_x, second_order_y;
				float Curvature;
				float t;
				t = 0.5;
				first_order_x = BezierFirstDerivat(Control_pts[4 * error_min_index + 0].x, Control_pts[4 * error_min_index + 1].x,
					Control_pts[4 * error_min_index + 2].x, Control_pts[4 * error_min_index + 3].x, t);
				first_order_y = BezierFirstDerivat(Control_pts[4 * error_min_index + 0].y, Control_pts[4 * error_min_index + 1].y,
					Control_pts[4 * error_min_index + 2].y, Control_pts[4 * error_min_index + 3].y, t);
				second_order_x = BezierSecondDerivat(Control_pts[4 * error_min_index + 0].x, Control_pts[4 * error_min_index + 1].x,
					Control_pts[4 * error_min_index + 2].x, Control_pts[4 * error_min_index + 3].x, t);
				second_order_y = BezierSecondDerivat(Control_pts[4 * error_min_index + 0].y, Control_pts[4 * error_min_index + 1].y,
					Control_pts[4 * error_min_index + 2].y, Control_pts[4 * error_min_index + 3].y, t);
				Curvature = BezierCurvature(first_order_x, second_order_x, first_order_y, second_order_y);
				curvative[i] = Curvature;


				optimal_control_pts[4 * i + 0] = Control_pts[4 * error_min_index + 0];
				optimal_control_pts[4 * i + 1] = Control_pts[4 * error_min_index + 1];
				optimal_control_pts[4 * i + 2] = Control_pts[4 * error_min_index + 2];
				optimal_control_pts[4 * i + 3] = Control_pts[4 * error_min_index + 3];

				//test
				curvative[i + 1] = curvative[i];
				curvative[i + 2] = curvative[i];
				curvative[i + 3] = curvative[i];
				curvative[i + 4] = curvative[i];
				optimal_control_pts[4 * (i + 1) + 0] = Control_pts[4 * error_min_index + 0];
				optimal_control_pts[4 * (i + 1) + 1] = Control_pts[4 * error_min_index + 1];
				optimal_control_pts[4 * (i + 1) + 2] = Control_pts[4 * error_min_index + 2];
				optimal_control_pts[4 * (i + 1) + 3] = Control_pts[4 * error_min_index + 3];
				optimal_control_pts[4 * (i + 2) + 0] = Control_pts[4 * error_min_index + 0];
				optimal_control_pts[4 * (i + 2) + 1] = Control_pts[4 * error_min_index + 1];
				optimal_control_pts[4 * (i + 2) + 2] = Control_pts[4 * error_min_index + 2];
				optimal_control_pts[4 * (i + 2) + 3] = Control_pts[4 * error_min_index + 3];
				optimal_control_pts[4 * (i + 3) + 0] = Control_pts[4 * error_min_index + 0];
				optimal_control_pts[4 * (i + 3) + 1] = Control_pts[4 * error_min_index + 1];
				optimal_control_pts[4 * (i + 3) + 2] = Control_pts[4 * error_min_index + 2];
				optimal_control_pts[4 * (i + 3) + 3] = Control_pts[4 * error_min_index + 3];
				optimal_control_pts[4 * (i + 4) + 0] = Control_pts[4 * error_min_index + 0];
				optimal_control_pts[4 * (i + 4) + 1] = Control_pts[4 * error_min_index + 1];
				optimal_control_pts[4 * (i + 4) + 2] = Control_pts[4 * error_min_index + 2];
				optimal_control_pts[4 * (i + 4) + 3] = Control_pts[4 * error_min_index + 3];

				if (Curvature > 1)
				{
					Curvature = 1;
					curvative[i] = 1;
					//test
					curvative[i + 1] = 1;
					curvative[i + 2] = 1;
					curvative[i + 3] = 1;
					curvative[i + 4] = 1;
				}
				mean_curvative += Curvature;
				if (Debug_mode == 1)
				{
					if (HistSectCount == SECTION_NUMBER)
					{
						//data_record_cnt = i;
						//if ((CurvePoint[HistSectCount][data_record_cnt].x - CurvePoint[HistSectCount][data_record_cnt_pre].x) >= 5)
						{
							fprintf(rfile1, "%.6f \n", curvative[i]);
							//data_record_cnt_pre = data_record_cnt;
						}
					}
					
				}
				i = i + 5; //i = i + 5
			}

			//if (Debug_mode==1)
			//	fprintf(rfile1,"%d \n",CurvePointCnt[HistSectCount]);
			if (CurvePointCnt[HistSectCount] > CURVE_LENGTH)
			{
				mean_curvative = mean_curvative / (CurvePointCnt[HistSectCount] - CURVE_LENGTH);
			}
			else
				mean_curvative = 0;
			//if (Debug_mode==1)
			//	fprintf(rfile1,"%.6f \n",mean_curvative);

			//找到局部峰值
			unsigned char start_flag = 0, stop_flag = 0;
			int start_index = 0;
			float accumulate_threshold = 3.0, accumulate_threshold_section = 3.0;
			CvPoint CurveDetectPoint[100];//曲率检测后的特征点
			int CurveDetectPointCnt = 0;    //曲率检测出来的特征点个数
			CurveDetectPointCnt = 0;
			accumulate_threshold = 10*mean_curvative;//阈值为均值的1.2倍
			if (HistSectCount == SECTION_NUMBER)
			{
				accumulate_threshold_section = accumulate_threshold;
			}
			for (int i = CURVE_LENGTH / 2; i<CurvePointCnt[HistSectCount] - CURVE_LENGTH / 2; i = i + 5)//i++  i=i+2
			{
				if ((curvative[i] <= accumulate_threshold) && (curvative[i + 5] > accumulate_threshold))//i+1
				{
					if (start_flag == 0)
					{
						start_flag = 1;
						start_index = i;
					}
				}
				if ((curvative[i] > accumulate_threshold) && (curvative[i + 5] <= accumulate_threshold))//i+1
				{
					if (start_flag == 1)
					{
						start_flag = 0;
						if ((i - start_index)>3)  //15
						{
							if ((CurvePoint[HistSectCount][(i + start_index) / 2].y < (FrameH - 5)) && (CurvePoint[HistSectCount][(i + start_index) / 2].y > 5) &&
								(CurvePoint[HistSectCount][(i + start_index) / 2].x < (FrameW - 5)) && (CurvePoint[HistSectCount][(i + start_index) / 2].x > 5))
							{
								if (curvative[(i + start_index) / 2] > 0.2) //0.2
								{
									CurveDetectPoint[CurveDetectPointCnt].x = CurvePoint[HistSectCount][(i + start_index) / 2].x;
									CurveDetectPoint[CurveDetectPointCnt].y = CurvePoint[HistSectCount][(i + start_index) / 2].y;
									CurveDetectPointCnt++;
									if ((optimal_control_pts[4 * ((i + start_index) / 2) + 0].y > optimal_control_pts[4 * ((i + start_index) / 2) + 1].y) &&
										(optimal_control_pts[4 * ((i + start_index) / 2) + 0].y > optimal_control_pts[4 * ((i + start_index) / 2) + 2].y) &&
										(optimal_control_pts[4 * ((i + start_index) / 2) + 3].y > optimal_control_pts[4 * ((i + start_index) / 2) + 1].y) &&
										(optimal_control_pts[4 * ((i + start_index) / 2) + 3].y > optimal_control_pts[4 * ((i + start_index) / 2) + 2].y))
									{
										circle(HistImageTest, CurvePoint[HistSectCount][(i + start_index) / 2], 4, Scalar(0, 255, 0), 2, 8, 0); //波峰点
										if (HistSectCount == SECTION_NUMBER)
										{
											Point pt1;
											pt1.x = CurvePoint[HistSectCount][(i + start_index) / 2].x;
											pt1.y = CurvePoint[HistSectCount][(i + start_index) / 2].y - SECTION_NUMBER*gray_down_4.rows/10;
												
											circle(HistImageSectionTest, pt1, 4, Scalar(0, 255, 0), 2, 8, 0); //波峰点

											//画四个控制点
											Point Cpt1, Cpt2, Cpt3, Cpt4;
											Cpt1.x = optimal_control_pts[4 * ((i + start_index) / 2) + 0].x;
											Cpt1.y = optimal_control_pts[4 * ((i + start_index) / 2) + 0].y - SECTION_NUMBER*gray_down_4.rows / 10;
											Cpt2.x = optimal_control_pts[4 * ((i + start_index) / 2) + 1].x;
											Cpt2.y = optimal_control_pts[4 * ((i + start_index) / 2) + 1].y - SECTION_NUMBER*gray_down_4.rows / 10;
											Cpt3.x = optimal_control_pts[4 * ((i + start_index) / 2) + 2].x;
											Cpt3.y = optimal_control_pts[4 * ((i + start_index) / 2) + 2].y - SECTION_NUMBER*gray_down_4.rows / 10;
											Cpt4.x = optimal_control_pts[4 * ((i + start_index) / 2) + 3].x;
											Cpt4.y = optimal_control_pts[4 * ((i + start_index) / 2) + 3].y - SECTION_NUMBER*gray_down_4.rows / 10;
											circle(HistImageSectionTest, Cpt1, 4, Scalar(0, 255, 255), 2, 8, 0);
											circle(HistImageSectionTest, Cpt2, 4, Scalar(255, 0, 255), 2, 8, 0);
											circle(HistImageSectionTest, Cpt3, 4, Scalar(255, 0, 255), 2, 8, 0);
											circle(HistImageSectionTest, Cpt4, 4, Scalar(0, 255, 255), 2, 8, 0);

											//画对应的Bezier曲线
											Control_pts_draw[0] = optimal_control_pts[4 * ((i + start_index) / 2) + 0];
											Control_pts_draw[0].y = Control_pts_draw[0].y - SECTION_NUMBER*gray_down_4.rows / 10;
											Control_pts_draw[1] = optimal_control_pts[4 * ((i + start_index) / 2) + 1];
											Control_pts_draw[1].y = Control_pts_draw[1].y - SECTION_NUMBER*gray_down_4.rows / 10;
											Control_pts_draw[2] = optimal_control_pts[4 * ((i + start_index) / 2) + 2];
											Control_pts_draw[2].y = Control_pts_draw[2].y - SECTION_NUMBER*gray_down_4.rows / 10;
											Control_pts_draw[3] = optimal_control_pts[4 * ((i + start_index) / 2) + 3];
											Control_pts_draw[3].y = Control_pts_draw[3].y - SECTION_NUMBER*gray_down_4.rows / 10;

											CvPoint3D32f *pControls = Control_pts_draw;
											CvPoint pt_now, pt_pre;
											for (int ii = 0; ii <= 50;ii++)
											{
												float u = (float)ii / 50;
												CvPoint3D32f newPt = Bernstein(u, pControls);
												pt_now.x = (int)newPt.x;
												pt_now.y = (int)newPt.y;

												if (ii > 0) line(HistImageSectionTest, pt_now, pt_pre, Scalar(255, 255, 0), 2);
												pt_pre = pt_now;
											}

										}
										Point peak_pt1;
										Point peak_pt2;
										peak_pt1.x = CurvePoint[HistSectCount][(i + start_index) / 2].x / 5;
										peak_pt1.y = local_hist_height*HistSectCount;
										peak_pt2.x = CurvePoint[HistSectCount][(i + start_index) / 2].x / 5;
										peak_pt2.y = local_hist_height*(HistSectCount + 1);
										line(gray_down_copy, peak_pt1, peak_pt2, Scalar(0, 255, 0), 2, 8, 0);

										SectData[HistSectCount].LocalPeakPosition[SectData[HistSectCount].LocalPeakNum++] = peak_pt1.x;
									}
									else if ((optimal_control_pts[4 * ((i + start_index) / 2) + 0].y < optimal_control_pts[4 * ((i + start_index) / 2) + 1].y) &&
										(optimal_control_pts[4 * ((i + start_index) / 2) + 0].y < optimal_control_pts[4 * ((i + start_index) / 2) + 2].y) &&
										(optimal_control_pts[4 * ((i + start_index) / 2) + 3].y < optimal_control_pts[4 * ((i + start_index) / 2) + 1].y) &&
										(optimal_control_pts[4 * ((i + start_index) / 2) + 3].y < optimal_control_pts[4 * ((i + start_index) / 2) + 2].y))
									{
										circle(HistImageTest, CurvePoint[HistSectCount][(i + start_index) / 2], 4, Scalar(0, 0, 255), 2, 8, 0); //波谷点
										if (HistSectCount == SECTION_NUMBER)
										{
											Point pt1;
											pt1.x = CurvePoint[HistSectCount][(i + start_index) / 2].x;
											pt1.y = CurvePoint[HistSectCount][(i + start_index) / 2].y - SECTION_NUMBER*gray_down_4.rows / 10;
											circle(HistImageSectionTest, pt1, 4, Scalar(0, 0, 255), 2, 8, 0); //波峰点
										}
									}
								}
							}
						}

					}
				}

			}
			if (Debug_mode == 1)
			{
				;
				//if (HistSectCount == SECTION_NUMBER)
					//fprintf(rfile1, "\t %.6f \n", accumulate_threshold_section);
			}
				
			
			delete[] curvative;
			delete[] accumulate;
			delete[] optimal_control_pts;
		}
		if (Debug_mode == 1)
		{
			imwrite("./result/HistImageTest.jpg", HistImageTest);
			imwrite("./result/HistImageSectionTest.jpg", HistImageSectionTest);
		}
#endif	

		//判断各个局部丝槽是否属于同一条丝槽，并给出最终的丝槽条数
		int SilkSlot_cnt = 0;

		CArray<int, int>SilkSlot;
		CArray<int, int>SilkSlotNum;
		int first_section_flag = 0;

		for (int i = 0; i<gray_down_4.cols;)
		{
			unsigned char flag = 0;
			int number = 0;
			for (int j = 0; j<10; j++)
			{
				for (int k = 0; k<SectData[j].LocalPeakNum; k++)
				{
					int temp;
					temp = abs(SectData[j].LocalPeakPosition[k] - i);
					if (temp == 0)
					{
						number++;
						flag = 1;
					}
				}

			}
			if (flag == 1)
			{
				SilkSlot.Add(i);
				SilkSlotNum.Add(number);
			}
			i = i + 1;
		}

		SilkSlot_cnt = SilkSlot.GetSize();

		//test
		int test_pos[100];
		int test_pos2[100];
		for (int kkk = 0; kkk<SilkSlot_cnt; kkk++)
		{
			test_pos[kkk] = SilkSlot.GetAt(kkk);
			test_pos2[kkk] = SilkSlotNum.GetAt(kkk);
		}

		if (SilkSlot_cnt == 2)
		{
			if (abs(SilkSlot.GetAt(1) - SilkSlot.GetAt(0))<3)
			{
				SilkSlot.RemoveAt(1);
				int new_num = 0;
				new_num = SilkSlotNum.GetAt(1) + SilkSlotNum.GetAt(0);
				SilkSlotNum.RemoveAt(1);
				SilkSlotNum.SetAt(0, new_num);
			}
		}
		int temp_cnt = 0, temp_d[100];
		if (SilkSlot_cnt >2)
		{
			for (int i = 0; i<SilkSlot.GetSize() - 2; i++)
			{
				if ((SilkSlot.GetAt(i + 2) - SilkSlot.GetAt(i))<5)
				{
					int new_num = 0;
					new_num = SilkSlotNum.GetAt(i + 2) + SilkSlotNum.GetAt(i + 1) + SilkSlotNum.GetAt(i);
					SilkSlot.RemoveAt(i + 2);
					SilkSlot.RemoveAt(i + 1);
					SilkSlotNum.RemoveAt(i + 2);
					SilkSlotNum.RemoveAt(i + 1);
					SilkSlotNum.SetAt(i, new_num);
				}
				else if ((SilkSlot.GetAt(i + 1) - SilkSlot.GetAt(i))<5)
				{
					int new_num = 0;
					new_num = SilkSlotNum.GetAt(i + 1) + SilkSlotNum.GetAt(i);
					SilkSlot.RemoveAt(i + 1);
					SilkSlotNum.RemoveAt(i + 1);
					SilkSlotNum.SetAt(i, new_num);
				}
			}
			int index = SilkSlot.GetSize() - 2;
			if (index >= 1)
			{
				if ((SilkSlot.GetAt(index + 1) - SilkSlot.GetAt(index))<5)
				{
					int new_num = 0;
					new_num = SilkSlotNum.GetAt(index + 1) + SilkSlotNum.GetAt(index);
					SilkSlot.RemoveAt(index + 1);
					SilkSlotNum.RemoveAt(index + 1);
					SilkSlotNum.SetAt(index, new_num);
				}
			}

		}

		//for test
		SilkSlot_cnt = SilkSlot.GetSize();
		for (int kkk = 0; kkk<SilkSlot_cnt; kkk++)
		{
			test_pos[kkk] = SilkSlot.GetAt(kkk);
			test_pos2[kkk] = SilkSlotNum.GetAt(kkk);
		}


		for (int i = 0; i<SilkSlot.GetSize(); i++)
		{
			if (SilkSlotNum.GetAt(i)<3) //丝槽段数
			{
				SilkSlot.RemoveAt(i);
				SilkSlotNum.RemoveAt(i);
				i = 0;
			}
		}

		if (SilkSlot.GetSize() == 1)
		{
			if (SilkSlotNum.GetAt(0) < 3)  //丝槽段数
			{
				SilkSlot.RemoveAt(0);
				SilkSlotNum.RemoveAt(0);
			}
		}

		//for test
		SilkSlot_cnt = SilkSlot.GetSize();
		for (int kkk = 0; kkk<SilkSlot_cnt; kkk++)
		{
			test_pos[kkk] = SilkSlot.GetAt(kkk);
			test_pos2[kkk] = SilkSlotNum.GetAt(kkk);
		}

		float error_sum = 0, error_mean = 0;
		int stripe_flag = 0;
		int stripe_width = 0;
		if (SilkSlot.GetSize() > 2)
		{
			for (int kkk = 0; kkk<SilkSlot.GetSize() - 2; kkk++)
			{
				float temp1 = 0, temp2 = 0;
				temp1 = (float)abs(SilkSlot.GetAt(kkk) - SilkSlot.GetAt(kkk + 1));
				temp2 = (float)abs(SilkSlot.GetAt(kkk + 1) - SilkSlot.GetAt(kkk + 2));
				if (temp1 > temp2)
				{
					if (temp1 > stripe_width)
					{
						stripe_width = temp1;
					}
				}
				else
				{
					if (temp2 > stripe_width)
					{
						stripe_width = temp2;
					}
				}
				error_sum += (float)abs(temp1 - temp2);
			}
			error_mean = error_sum / float(SilkSlot.GetSize() - 2);
			if ((error_mean < 2.5) && (stripe_width > 6)) //stripe_width条纹的宽度
			{
				stripe_flag = 1;
			}
		}

		//m_iEditSilkNum = SilkSlot.GetSize();
		if (Debug_mode == 1)
			imwrite("./result/丝槽处理结果.jpg", gray_down_copy);
		int HoleNum = 0;
		int ErrorSize = 0;
		int ErrorSize_Threshold = 50;
		int Error_Num = 0;
		int min_error_size = 1000;
		string result;

		nowtime = GetTickCount();
		alg_time_cnt = nowtime - lastime;

		//m_iEditTime = alg_time_cnt;
		//m_iEditWhitePointNum = Error_Num; //WhitePointNum
		UpdateData(FALSE);

		CRect rect;
		Mat imgSrcPreview = Mat::zeros(imgSrc1.size(), CV_8UC3);
		GetDlgItem(IDC_STATIC_IMAGE_PROCESS)->GetClientRect(&rect);  // 获取控件大小
		cv::resize(imgSrcPreview, imgSrcPreview, cv::Size(rect.Width(), rect.Height()));// 缩小或放大Mat并备份

		double resize_ratio, resize_ratio_x, resize_ratio_y;

		Mat MaskModel;
		int temp_cols;
		int temp_rows;
		Rect roi_Rect_preview;

		imgSrc1 = gray_down_copy.clone();
		
		DrawMat(imgSrc1, IDC_STATIC_IMAGE_PROCESS);
		/*
		if (Folder_Flag == 1)
		{
			if (m_iEditSilkNum == 0 && Error_Num == 0)
			{
				result = "OK";
				OK_Num++;
				min_error_size = 0;
			}
			else
			{
				result = "NG";
				NG_Num++;
			}
			fprintf(rfile, "%d \t %s \t %s \t %d \t %d \t %d \n", image_cnt, file_name.c_str(), result.c_str(), m_iEditSilkNum, Error_Num, min_error_size);
		}
		*/

	}
	fprintf(rfile, "OK_Num \t NG_Num  \t \n");
	fprintf(rfile, "%d \t %d  \t \n", OK_Num, NG_Num);
	fclose(rfile);
	fclose(rfile1);
	fclose(rfile2);
	
	return result;
}
