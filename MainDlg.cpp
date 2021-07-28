// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"
#include <time.h>

int CMainDlg::audioCallback(void * ctx, void * buf, int len)
{
	CMainDlg * the_obj = (CMainDlg *)ctx;
	the_obj->onAudioData(buf, len);
	return 0;
}
void CMainDlg::onAudioData(void * buf, int len)
{
	float * p_sample = (float *)buf;
	int sample_num = len / 4;
	int tn = 1024;//每多少个样本显示一次
	static float sum = 0.0;
	static float max = 0.0;
	static int nn = 0;
	for (int i = 0; i < sample_num; i++) {
		sum += p_sample[i];
		if (p_sample[i] > max)
			max = p_sample[i];
		nn++;
		if (nn > tn) {
			nn = 0;
			sum = sum / tn;
			level = sum;
			level_max = max;	
			PostMessage(WM_SHOWWAVE);
			//waveshow_do(waveshow_ctx, &sum, sample_num);
			sum = 0;
			max = 0.0;
		}
	}
	if (_b_rec == 1) {
		int rt = mp3_encode_do(enc_ctx, p_sample, len/4/_channel_num);
		if (rt != 0)
			_enc_err = 1;
		else
			_enc_err = 0;
	}
}

void CMainDlg::DrawWaveBackground(HDC hDC)
{
	int w = waveWin_width;
	int h = waveWin_height;

	HBRUSH hBrush = CreateSolidBrush(RGB(0, 30, 60)); // 创建蓝色画刷
	//SelectObject(hMemDC_1, hBrush);  // 指定画刷
	//Rectangle(hMemDC_1, 0, 0, w, h);
	RECT rect = { 0,0,w,h };
	FillRect(hDC, &rect, hBrush);
	HPEN  hPen = ::CreatePen(PS_SOLID, 2, RGB(0, 50, 0));	//创建一支红色的画笔
	SelectObject(hDC, hPen);		//将画板放入画板

	for (int i = 0; i < 16; i++) {
		int x = i * w / 16;
		::MoveToEx(hDC, x, 0, NULL);
		::LineTo(hDC, x, h);
	}
	for (int i = 0; i < 6; i++) {
		int y = i * h / 6;
		::MoveToEx(hDC, 0, y, NULL);
		::LineTo(hDC, w, y);
	}

	::DeleteObject(hPen);
	::DeleteObject(hBrush);
}

LRESULT CMainDlg::OnShowWave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	//HDC hDC = ::GetDC(hWave);
	int w = waveWin_width;
	int h = waveWin_height;
	static int pos = 0;

	//DrawWaveBackground(hMemDC_1);
	int sx = pos;
	int bw = w - pos;
	if (bw > 0) {
		BitBlt(hDC_wave, 0, 0, bw, h, hMemDC_1, sx, 0, SRCCOPY);
	}
	int dx = w-pos + 1;
	bw = pos;
	if (bw > 0) {
		HPEN  hPen = ::CreatePen(PS_SOLID, 2, RGB(0, 255, 0));	//创建一支红色的画笔
		SelectObject(hMemDC_2, hPen);		//将画板放入画板
		int y =(h- h * level_max)/ 2;
		::MoveToEx(hMemDC_2, pos, y, NULL);
		::LineTo(hMemDC_2, pos, h-y);
		BitBlt(hDC_wave, dx, 0, bw, h, hMemDC_2, 0, 0, SRCCOPY);
		::DeleteObject(hPen);
	}
	pos++;
	if (pos > w) {
		pos = 0;
		HDC tmp = hMemDC_1;
		hMemDC_1 = hMemDC_2;
		hMemDC_2 = tmp;
		DrawWaveBackground(hMemDC_2);
	}

	//在画板上面写文字
	if (_b_rec == 1) {
		::SetBkMode(hDC_wave, TRANSPARENT);		//画板背景透明
		::SetTextColor(hDC_wave, RGB(250, 0, 0));	//设置画板文字颜色
		::TextOut(hDC_wave, 20, 20, _T("正在录音"), lstrlenW(_T("正在录音")));	//写文字
	}
	return 0;
}
BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
	UIUpdateChildWindows();
	return FALSE;
}

//开启音频捕获
int CMainDlg::startCapture()
{
	int ret = 0;
	GetDlgItem(IDC_BTN_REC).EnableWindow(0);
	WasapiDevice dev = devicesList[dev_index];
	if (dev.is_default) {
		ret = audio_capture_creat(&capture_ctx, NULL, dev.is_input);
	}
	else {
		ret = audio_capture_creat(&capture_ctx, dev.deviceID, dev.is_input);
	}
	if (ret != NULL)
		return -1;
	audio_capture_setcallback(capture_ctx, audioCallback, this);
	_sample_rate = 0;
	_channel_num = 0;
	audio_capture_getFormat(capture_ctx, &_sample_rate, &_channel_num);
	if (_sample_rate == 0 || _channel_num == 0)
		return -1;
	audio_capture_start(capture_ctx);
	GetDlgItem(IDC_BTN_REC).EnableWindow(1);
	return 0;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();
	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	UIAddChildWindowContainer(m_hWnd);

	HICON hIcon_NULL= ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_NULL));
	
	WasapiDevice dev;
	dev.is_input = 0;
	dev.is_default = 1;
	wcscpy(dev.descText,L"电脑输出   ：(默认设备)");
	devicesList.push_back(dev);
	dev.is_input = 1;
	dev.is_default = 1;
	wcscpy(dev.descText , L"麦克风输入 ：(默认设备)");
	devicesList.push_back(dev);

	GetWASAPIAudioDevices(devicesList);
	
	GetDlgItem(IDC_BTN_STOP).EnableWindow(0);

	CButton devBTN = (CButton)GetDlgItem(IDC_COMMAND_DEVICE);
	HICON hIcon_mic = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_MIC));
	devBTN.SetIcon(hIcon_mic);
	devBTN.SetWindowTextW(devicesList[0].descText);

	CButton pathBTN = (CButton)GetDlgItem(IDC_COMMAND_PATH);

	wchar_t path[1024];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_MYMUSIC, NULL, SHGFP_TYPE_CURRENT, path);
	pathBTN.SetWindowTextW(path);
	//p.SetButtonStyle(p.GetButtonStyle() ^ BS_ICON);
	pathBTN.SetNote(_T("文件保存位置，点击打开文件夹"));  //设置注意是的内容
	HICON hIcon_path = ::LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_PATH));
	pathBTN.SetIcon(hIcon_path);  //设置icon,当然也可以使用默认的icon.

	CButton bitrateBTN = (CButton)GetDlgItem(IDC_COMMAND_BITRATE);
	bitrateBTN.SetNote(_T("mp3比特率"));
	bitrateBTN.SetIcon(hIcon_NULL);

	CButton setpathBTN = (CButton)GetDlgItem(IDC_COMMAND_SETPATH);
	setpathBTN.SetIcon(hIcon_NULL);

	CButton deviceBTN = (CButton)GetDlgItem(IDC_COMMAND_DEVICE);
	deviceBTN.SetWindowText(devicesList[dev_index].descText);
	deviceBTN.SetNote(_T("录音设备  点击更改"));

	//GetDlgItem(IDOK);
	HWND hWave = ::GetDlgItem(m_hWnd, IDC_STATIC_WAVE);

	RECT rect;
	::GetWindowRect(hWave, &rect);
	waveWin_width = rect.right - rect.left;
	waveWin_height = rect.bottom - rect.top;

	hDC_wave = ::GetDC(hWave);
	hMemDC_1 = CreateCompatibleDC(hDC_wave);
	hMemDC_2 = CreateCompatibleDC(hDC_wave);
	HBITMAP hBmp1 = CreateCompatibleBitmap(hDC_wave, waveWin_width, waveWin_height);
	HBITMAP hBmp2 = CreateCompatibleBitmap(hDC_wave, waveWin_width, waveWin_height);
	SelectObject(hMemDC_1, hBmp1);
	SelectObject(hMemDC_2, hBmp2);
	DrawWaveBackground(hMemDC_1);
	DrawWaveBackground(hMemDC_2);

	//打开音频捕获
	if (startCapture() != 0) {
		MessageBox(L"音频捕获出错");
		GetDlgItem(IDC_BTN_REC).EnableWindow(0);
	}
	else
		GetDlgItem(IDC_BTN_REC).EnableWindow(1);
	return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	return 0;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code 
	CloseDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL b;
	if (_b_rec == 1)
		OnBnClickedBtnStop(0, 0, 0, b);
	if (capture_ctx != NULL) {
		audio_capture_stop(capture_ctx);
		audio_capture_free(&capture_ctx);
	}
	CloseDialog(wID);
	return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}


LRESULT CMainDlg::OnBnClickedBtnRec(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	wchar_t path[1024];
	GetDlgItem(IDC_COMMAND_PATH).GetWindowText(path, 1024);

	time_t timer;
	struct tm *Now;
	time(&timer);
	Now = localtime(&timer);
	wchar_t name[1024];
	wsprintfW(name, L"%s\\%02d-%02d-%02d-%02d-%02d.mp3",path,Now->tm_mon,Now->tm_mday,Now->tm_hour,Now->tm_min,Now->tm_sec );

	GetDlgItem(IDC_BTN_STOP).EnableWindow(1);
	GetDlgItem(IDC_BTN_REC).EnableWindow(0);
	GetDlgItem(IDC_COMMAND_DEVICE).EnableWindow(0);
	enc_ctx = mp3_encode_open(name, _sample_rate, _channel_num, 0, _bit_rate);
	if (enc_ctx != NULL)
		_b_rec = 1;
	else {
		GetDlgItem(IDC_BTN_STOP).EnableWindow(0);
		GetDlgItem(IDC_BTN_REC).EnableWindow(1);
		GetDlgItem(IDC_COMMAND_DEVICE).EnableWindow(1);
		MessageBox(L"编码器打开出错", L"录音出错");
	}
	return 0;
}


LRESULT CMainDlg::OnBnClickedBtnStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BTN_STOP).EnableWindow(0);
	GetDlgItem(IDC_BTN_REC).EnableWindow(1);
	GetDlgItem(IDC_COMMAND_DEVICE).EnableWindow(1);
	_b_rec = 0;
	Sleep(100);
	mp3_encode_close(enc_ctx);
	enc_ctx = NULL;
	return 0;
}


LRESULT CMainDlg::OnBnClickedCommandPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//ShellExecute(NULL, L"open", L"http://www.neu.edu.cn", NULL, NULL, SW_SHOWNORMAL);
	wchar_t path[1024];
	GetDlgItem(IDC_COMMAND_PATH).GetWindowText(path,1024);
	ShellExecute(NULL, L"open", path, NULL, NULL, SW_SHOWNORMAL);
	return 0;
}


LRESULT CMainDlg::OnBnClickedCommandSetpath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	CFolderDialog dlg(m_hWnd);
	int rt = dlg.DoModal();
	if (rt == IDOK) {
		LPCTSTR path = dlg.GetFolderPath();
		GetDlgItem(IDC_COMMAND_PATH).SetWindowText(path);
	}
	return 0;
}

LRESULT CMainDlg::OnBnClickedCommandDevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码
	//#define  ITEM_MENU_REPLY  40001
	int firstID = 40001;
	HMENU hPopMenu = CreatePopupMenu();
	int n = devicesList.size();
	for (int i = 0; i < n; i++) {
		::InsertMenu(hPopMenu, (-1), MF_BYPOSITION, firstID + i, devicesList[i].descText);
	}
	POINT p;
	GetCursorPos(&p);
	int ret = TrackPopupMenu(hPopMenu, TPM_RETURNCMD, p.x, p.y,0, m_hWnd, NULL);
	int index = ret - firstID;
	if (ret != 0 && index != dev_index) {
		dev_index = index;
		CButton deviceBTN = (CButton)GetDlgItem(IDC_COMMAND_DEVICE);
		deviceBTN.SetWindowText(devicesList[dev_index].descText);
		if (capture_ctx != NULL) {
			audio_capture_stop(capture_ctx);
			audio_capture_free(&capture_ctx);
		}
		startCapture();
	}

	DestroyMenu(hPopMenu);
	return 0;
}


LRESULT CMainDlg::OnBnClickedCommandBitrate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int firstID = 40001;
	HMENU hPopMenu = CreatePopupMenu();
	::InsertMenu(hPopMenu, (-1), MF_BYPOSITION, firstID + 0, TEXT("128K bps"));
	::InsertMenu(hPopMenu, (-1), MF_BYPOSITION, firstID + 1, TEXT("192K bps"));
	::InsertMenu(hPopMenu, (-1), MF_BYPOSITION, firstID + 2, TEXT("224K bps"));
	::InsertMenu(hPopMenu, (-1), MF_BYPOSITION, firstID + 3, TEXT("320K bps"));


	POINT p;
	GetCursorPos(&p);
	int ret = TrackPopupMenu(hPopMenu, TPM_RETURNCMD, p.x, p.y, 0, m_hWnd, NULL);
	if (ret == firstID + 0) {
		_bit_rate = 128000;
		GetDlgItem(IDC_COMMAND_BITRATE).SetWindowTextW(TEXT("128K bps"));
	}
	else if (ret == firstID + 1) {
		_bit_rate = 192000;
		GetDlgItem(IDC_COMMAND_BITRATE).SetWindowTextW(TEXT("192K bps"));
	}
	else if (ret == firstID + 2) {
		_bit_rate = 224000;
		GetDlgItem(IDC_COMMAND_BITRATE).SetWindowTextW(TEXT("224K bps"));
	}
	else if (ret == firstID + 3) {
		_bit_rate = 320000;
		GetDlgItem(IDC_COMMAND_BITRATE).SetWindowTextW(TEXT("320K bps"));
	}

	DestroyMenu(hPopMenu);

	return 0;
}


LRESULT CMainDlg::OnNMClickSyslinkabout(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}
