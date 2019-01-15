
// RunAppContainerDlg.h : header file
//

#pragma once


// CRunAppContainerDlg dialog
class CRunAppContainerDlg : public CDialogEx
{
// Construction
public:
	CRunAppContainerDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RUNAPPCONTAINER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	bool ExecuteAppContainer(const CString& containerName, CString& exePath, const CString& files, const CString& registry, CString& log);
	static bool AllowNamedObjectAccess(PSID appContainerSid, PWSTR name, SE_OBJECT_TYPE type, ACCESS_MASK accessMask);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedRun();
};
