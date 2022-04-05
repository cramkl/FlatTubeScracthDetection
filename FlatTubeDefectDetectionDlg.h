
// FlatTubeDefectDetectionDlg.h : 头文件
//

#pragma once

#include <vector>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp>  
#include <highgui.h>
#include <opencv.hpp>
#include <string.h>

using namespace cv;
using namespace std;

struct PARAMETERS
{
	int Threshold1;
	int Threshold2;
	int Flag;
};

// CFlatTubeDefectDetectionDlg 对话框
class CFlatTubeDefectDetectionDlg : public CDialogEx
{
// 构造
public:
	CFlatTubeDefectDetectionDlg(CWnd* pParent = NULL);	// 标准构造函数

	void DrawMat(cv::Mat& img, UINT nID);
	Mat ImageProcess(cv::Mat& img, PARAMETERS para);

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FLATTUBEDEFECTDETECTION_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_StrEditFilePath;
	afx_msg void OnBnClickedBtnOpenFile();
	afx_msg void OnBnClickedBtnProcessSilk();
};
