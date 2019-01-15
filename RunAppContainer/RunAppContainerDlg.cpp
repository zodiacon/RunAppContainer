
// RunAppContainerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RunAppContainer.h"
#include "RunAppContainerDlg.h"

#pragma comment(lib, "userenv")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx {
public:
	CAboutDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRunAppContainerDlg dialog



CRunAppContainerDlg::CRunAppContainerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RUNAPPCONTAINER_DIALOG, pParent) {
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRunAppContainerDlg::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRunAppContainerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BROWSE, &CRunAppContainerDlg::OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_RUN, &CRunAppContainerDlg::OnBnClickedRun)
END_MESSAGE_MAP()


// CRunAppContainerDlg message handlers

BOOL CRunAppContainerDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr) {
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRunAppContainerDlg::OnSysCommand(UINT nID, LPARAM lParam) {
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRunAppContainerDlg::OnPaint() {
	if (IsIconic()) {
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else {
		CDialogEx::OnPaint();
	}
}

HCURSOR CRunAppContainerDlg::OnQueryDragIcon() {
	return static_cast<HCURSOR>(m_hIcon);
}


void CRunAppContainerDlg::OnBnClickedBrowse() {
	CFileDialog dlg(TRUE, L"exe", nullptr, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER, L"Executables (*.exe)|*.exe||", this);
	if (dlg.DoModal() == IDOK)
		SetDlgItemText(IDC_EXEPATH, dlg.GetPathName());
}


void CRunAppContainerDlg::OnBnClickedRun() {
	CString containerName;
	GetDlgItemText(IDC_NAME, containerName);
	if (containerName.IsEmpty()) {
		AfxMessageBox(L"Container name cannot be empty");
		return;
	}

	CString exePath;
	GetDlgItemText(IDC_EXEPATH, exePath);
	if (exePath.IsEmpty()) {
		AfxMessageBox(L"Executable path must be specified");
		return;
	}

	CString files, registry;
	GetDlgItemText(IDC_FILES, files);
	GetDlgItemText(IDC_REGISTRY, registry);

	CString log;
	GetDlgItemText(IDC_INFO, log);
	bool ok = ExecuteAppContainer(containerName, exePath, files, registry, log);
	SetDlgItemText(IDC_INFO, log);
	if (!ok)
		AfxMessageBox(L"Failed!");
}

bool CRunAppContainerDlg::AllowNamedObjectAccess(PSID appContainerSid, PWSTR name, SE_OBJECT_TYPE type, ACCESS_MASK accessMask) {
	PACL oldAcl, newAcl = nullptr;
	DWORD status;
	EXPLICIT_ACCESS access;
	do {
		access.grfAccessMode = GRANT_ACCESS;
		access.grfAccessPermissions = accessMask;
		access.grfInheritance = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;
		access.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
		access.Trustee.pMultipleTrustee = nullptr;
		access.Trustee.ptstrName = (PWSTR)appContainerSid;
		access.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		access.Trustee.TrusteeType = TRUSTEE_IS_GROUP;

		status = GetNamedSecurityInfo(name, type, DACL_SECURITY_INFORMATION, nullptr, nullptr, &oldAcl, nullptr, nullptr);
		if (status != ERROR_SUCCESS)
			return false;

		status = SetEntriesInAcl(1, &access, oldAcl, &newAcl);
		if (status != ERROR_SUCCESS)
			return false;

		status = SetNamedSecurityInfo(name, type, DACL_SECURITY_INFORMATION, nullptr, nullptr, newAcl, nullptr);
		if (status != ERROR_SUCCESS)
			break;
	} while (false);

	if (newAcl)
		::LocalFree(newAcl);

	ASSERT(status == ERROR_SUCCESS);
	return status == ERROR_SUCCESS;
}

bool CRunAppContainerDlg::ExecuteAppContainer(const CString & containerName, CString & exePath, const CString & files, const CString & registry, CString& log) {
	PSID appContainerSid;
	auto hr = ::CreateAppContainerProfile(containerName, containerName, containerName, nullptr, 0, &appContainerSid);
	if (FAILED(hr)) {
		// see if AppContainer SID already exists
		hr = ::DeriveAppContainerSidFromAppContainerName(containerName, &appContainerSid);
		if (FAILED(hr))
			return false;
	}

	PWSTR str;
	::ConvertSidToStringSid(appContainerSid, &str);
	((log += L"AppContainer SID:\r\n") += str) += L"\r\n";

	PWSTR path;
	if (SUCCEEDED(::GetAppContainerFolderPath(str, &path))) {
		((log += L"AppContainer folder: ") += path) += L"\r\n";
		::CoTaskMemFree(path);
	}
	::LocalFree(str);

	// build process attributes
	// for simplicity (for now), have just one  capabilities

	SECURITY_CAPABILITIES sc = { 0 };
	sc.AppContainerSid = appContainerSid;

	/*
	SID_AND_ATTRIBUTES cap;
	BYTE sidBuffer[SECURITY_MAX_SID_SIZE];
	PSID inetCapability = reinterpret_cast<PSID>(sidBuffer);
	DWORD sizeSid;
	if (!CreateWellKnownSid(WinCapabilityInternetClientSid, nullptr, inetCapability, &sizeSid))
		return false;

	cap.Sid = inetCapability;
	cap.Attributes = SE_GROUP_ENABLED;
	*/

	STARTUPINFOEX si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	SIZE_T size;

	::InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
	auto buffer = std::make_unique<BYTE[]>(size);
	si.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(buffer.get());
	if (!::InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &size))
		return false;
	if (!::UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES, &sc, sizeof(sc), nullptr, nullptr))
		return false;

	// set security for files/folders

	int start = 0;
	do {
		auto filename = files.Tokenize(L"\r\n", start);
		if (filename.IsEmpty())
			break;
		AllowNamedObjectAccess(appContainerSid, filename.GetBuffer(), SE_FILE_OBJECT, FILE_ALL_ACCESS);
	} while (true);

	start = 0;
	do {
		auto filename = registry.Tokenize(L"\r\n", start);
		if (filename.IsEmpty())
			break;
		AllowNamedObjectAccess(appContainerSid, filename.GetBuffer(), SE_REGISTRY_WOW64_32KEY, KEY_ALL_ACCESS);
	} while (true);

	BOOL created = ::CreateProcess(nullptr, exePath.GetBuffer(), nullptr, nullptr, FALSE,
		EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr, (LPSTARTUPINFO)&si, &pi);
	DeleteProcThreadAttributeList(si.lpAttributeList);

	if (!created)
		return false;

	CString text;
	text.Format(L"Created process %d\r\n", pi.dwProcessId);
	log += text;

	return true;
}
