/*

WhatsApp plugin for Miranda NG
Copyright � 2019-25 George Hazan

*/

#include "stdafx.h"

class CWhatsAppQRDlg : public CProtoDlgBase<WhatsAppProto>
{
public:
	CWhatsAppQRDlg(WhatsAppProto *ppro) :
		CProtoDlgBase<WhatsAppProto>(ppro, IDD_SHOWQR)
	{}

	void OnDestroy() override
	{
		m_proto->m_pQRDlg = nullptr;
		
		if (!m_bSucceeded)
			m_proto->ShutdownSession();
	}

	void SetSuccess()
	{
		m_bSucceeded = true;
	}

	void SetData(const CMStringA &str)
	{
		auto *pQR = QRcode_encodeString(str, 0, QR_ECLEVEL_L, QR_MODE_8, 1);

		HWND hwndRc = GetDlgItem(m_hwnd, IDC_QRPIC);
		RECT rc;
		GetClientRect(hwndRc, &rc);

		::SetForegroundWindow(m_hwnd);

		int scale = 8; // (rc.bottom - rc.top) / pQR->width;
		int rowLen = pQR->width * scale * 3;
		if (rowLen % 4)
			rowLen = (rowLen / 4 + 1) * 4;
		int dataLen = rowLen * pQR->width * scale;

		mir_ptr<BYTE> pData((BYTE *)mir_alloc(dataLen));
		if (pData == nullptr) {
			QRcode_free(pQR);
			return;
		}

		memset(pData, 0xFF, dataLen); // white background by default

		const BYTE *s = pQR->data;
		for (int y = 0; y < pQR->width; y++) {
			BYTE *d = pData.get() + rowLen * y * scale;
			for (int x = 0; x < pQR->width; x++) {
				if (*s & 1)
					for (int i = 0; i < scale; i++)
						for (int j = 0; j < scale; j++) {
							d[j * 3 + i * rowLen] = 0;
							d[1 + j * 3 + i * rowLen] = 0;
							d[2 + j * 3 + i * rowLen] = 0;
						}

				d += scale * 3;
				s++;
			}
		}

		BITMAPFILEHEADER fih = {};
		fih.bfType = 0x4d42;  // "BM"
		fih.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dataLen;
		fih.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		BITMAPINFOHEADER bih = {};
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = pQR->width * scale;
		bih.biHeight = -bih.biWidth;
		bih.biPlanes = 1;
		bih.biBitCount = 24;
		bih.biCompression = BI_RGB;

		wchar_t wszTempPath[MAX_PATH], wszTempFile[MAX_PATH];
		GetTempPathW(_countof(wszTempPath), wszTempPath);
		GetTempFileNameW(wszTempPath, L"wa_", TRUE, wszTempFile);
		FILE *f = _wfopen(wszTempFile, L"wb");
		fwrite(&fih, sizeof(BITMAPFILEHEADER), 1, f);
		fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, f);
		fwrite(pData, sizeof(unsigned char), dataLen, f);
  		fclose(f);

		SendMessage(hwndRc, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)Image_Load(wszTempFile));
		
		DeleteFileW(wszTempFile);
		QRcode_free(pQR);
	}
};

static INT_PTR __stdcall sttShowDialog(void *param)
{
	WhatsAppProto *ppro = (WhatsAppProto *)param;

	if (ppro->m_pQRDlg == nullptr) {
		ppro->m_pQRDlg = new CWhatsAppQRDlg(ppro);
		ppro->m_pQRDlg->Show();
	}
	else {
		SetForegroundWindow(ppro->m_pQRDlg->GetHwnd());
		SetActiveWindow(ppro->m_pQRDlg->GetHwnd());
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::CloseQrDialog()
{
	if (m_pQRDlg) {
		m_pQRDlg->SetSuccess();
		m_pQRDlg->Close();
	}
}

bool WhatsAppProto::ShowQrCode(const CMStringA &ref)
{
	CallFunctionSync(sttShowDialog, this);

	MBinBuffer secret(getBlob(DBKEY_SECRET_KEY));

	ptrA s1(mir_base64_encode(m_noise->noiseKeys.pub));
	ptrA s2(mir_base64_encode(m_signalStore.signedIdentity.pub));
	ptrA s3(mir_base64_encode(secret));
	CMStringA szQrData(FORMAT, "%s,%s,%s,%s", ref.c_str(), s1.get(), s2.get(), s3.get());
	m_pQRDlg->SetData(szQrData);
	return true;
}
