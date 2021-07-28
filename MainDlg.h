// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "wasapi_capture.h"

using namespace std;

#define WM_SHOWWAVE		WM_USER+200

struct MP3ENC_CTX;
MP3ENC_CTX * mp3_encode_open(wchar_t * name, int rate, int ch, int format, int bit_rate);
int mp3_encode_close(MP3ENC_CTX * ctx);
int mp3_encode_do(MP3ENC_CTX * ctx, float * samples, int sample_num);

int GetWASAPIAudioDevices(vector<WasapiDevice> &devicesList);

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
		public CMessageFilter, public CIdleHandler
{
public:
	enum { IDD = IDD_MAINDLG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SHOWWAVE, OnShowWave)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_BTN_REC, BN_CLICKED, OnBnClickedBtnRec)
		COMMAND_HANDLER(IDC_BTN_STOP, BN_CLICKED, OnBnClickedBtnStop)
		COMMAND_HANDLER(IDC_COMMAND_PATH, BN_CLICKED, OnBnClickedCommandPath)
		COMMAND_HANDLER(IDC_COMMAND_SETPATH, BN_CLICKED, OnBnClickedCommandSetpath)
		COMMAND_HANDLER(IDC_COMMAND_DEVICE, BN_CLICKED, OnBnClickedCommandDevice)
		COMMAND_HANDLER(IDC_COMMAND_BITRATE, BN_CLICKED, OnBnClickedCommandBitrate)
		NOTIFY_HANDLER(IDC_SYSLINK_about, NM_CLICK, OnNMClickSyslinkabout)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnShowWave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	void CloseDialog(int nVal);
	LRESULT OnBnClickedBtnRec(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	
	vector<WasapiDevice> devicesList;
	int dev_index = 0;
	AUDIO_CAPTURE_CTX * capture_ctx = NULL;
	int startCapture();
	

	int _sample_rate = 48000;
	int _channel_num = 2;
	int _bit_rate = 192000;

	MP3ENC_CTX *enc_ctx;
	int _b_rec = 0;
	int _enc_err = 0;

	void * waveshow_ctx;
	static int audioCallback(void * ctx, void * buf, int len);
	void onAudioData(void * buf, int len);
	float level = 0;
	float level_max = 0;
	HDC hDC_wave;
	HDC hMemDC_1;
	HDC hMemDC_2;
	int waveWin_width;
	int waveWin_height;
	void DrawWaveBackground(HDC hDC);
	LRESULT OnBnClickedBtnStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCommandPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCommandSetpath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCommandDevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCommandBitrate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNMClickSyslinkabout(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
};
