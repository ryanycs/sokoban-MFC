
// Sokoban.h : main header file for the Sokoban application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CSokobanApp:
// See Sokoban.cpp for the implementation of this class
//

class CSokobanApp : public CWinApp
{
public:
	CSokobanApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CSokobanApp theApp;
