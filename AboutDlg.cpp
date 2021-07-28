// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"

wchar_t * text1 =
L"索存良\r\n"
L"13731037077\r\n"
L"电话微信同步\r\n"
L"tinysuo@163.com";
wchar_t * text2 = 
L"mp3编码器使用了lame\r\n"
L"音频捕获参考了OBS\r\n"
L"图标来源于网络";
wchar_t * text3 = 
L"特点：\r\n"
L"1、支持电脑内录，录制电脑播放的音频；\r\n"
L"2、体积小，绿色单文件；\r\n"
L"3、开源免费，无广告，无流氓行为；\r\n";

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());
	GetDlgItem(IDC_STATIC_text1).SetWindowTextW(text1);
	GetDlgItem(IDC_STATIC_text2).SetWindowTextW(text2);
	GetDlgItem(IDC_STATIC_text3).SetWindowTextW(text3);

	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
LRESULT CAboutDlg::OnNMClickSyslink1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	ShellExecute(NULL, L"open", L"https://github.com/bitcat-live/bitRec", NULL, NULL, SW_SHOWNORMAL);
	return 0;
}
LRESULT CAboutDlg::OnNMClickSyslink3(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	ShellExecute(NULL, L"open", L"https://www.bitcat.live/", NULL, NULL, SW_SHOWNORMAL);
	return 0;
}
LRESULT CAboutDlg::OnNMClickSyslink2(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	ShellExecute(NULL, L"open", L"https://www.bitcat.live/bitRec/", NULL, NULL, SW_SHOWNORMAL);
	return 0;
}
