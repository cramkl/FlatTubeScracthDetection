
// FlatTubeDefectDetection.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CFlatTubeDefectDetectionApp: 
// �йش����ʵ�֣������ FlatTubeDefectDetection.cpp
//

class CFlatTubeDefectDetectionApp : public CWinApp
{
public:
	CFlatTubeDefectDetectionApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CFlatTubeDefectDetectionApp theApp;