// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"

wchar_t * text1 =
L"������\r\n"
L"13731037077\r\n"
L"�绰΢��ͬ��\r\n"
L"tinysuo@163.com";
wchar_t * text2 = 
L"mp3������ʹ����lame\r\n"
L"��Ƶ����ο���OBS\r\n"
L"ͼ����Դ������";
wchar_t * text3 = 
L"�ص㣺\r\n"
L"1��֧�ֵ�����¼��¼�Ƶ��Բ��ŵ���Ƶ��\r\n"
L"2�����С����ɫ���ļ���\r\n"
L"3����Դ��ѣ��޹�棬����å��Ϊ��\r\n";

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
