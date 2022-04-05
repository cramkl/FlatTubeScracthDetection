
// FlatTubeDefectDetectionDlg.h : ͷ�ļ�
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

// CFlatTubeDefectDetectionDlg �Ի���
class CFlatTubeDefectDetectionDlg : public CDialogEx
{
// ����
public:
	CFlatTubeDefectDetectionDlg(CWnd* pParent = NULL);	// ��׼���캯��

	void DrawMat(cv::Mat& img, UINT nID);
	Mat ImageProcess(cv::Mat& img, PARAMETERS para);

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FLATTUBEDEFECTDETECTION_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
