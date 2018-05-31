// display a bitmap
void SetSplashImage(HWND hwndSplash, HBITMAP hbmpSplash, int offset, int offsety, bool setSize = true) {
	// get the size of the bitmap
	BITMAP bm;
	GetObject(hbmpSplash, sizeof(bm), &bm);
	SIZE _sizeSplash = { bm.bmWidth, bm.bmHeight };

	ptOrigin.x = offset;
	ptOrigin.y = offsety;

	// create a memory DC holding the splash bitmap
	g_hdcScreen = GetDC(NULL);
	g_hdcMem = CreateCompatibleDC(g_hdcScreen);
	HBITMAP hbmpOld = (HBITMAP)SelectObject(g_hdcMem, hbmpSplash);

	// use the source image's alpha channel for blending
	g_blend = { 0 };
	g_blend.BlendOp = AC_SRC_OVER;
	g_blend.SourceConstantAlpha = 255;
	g_blend.AlphaFormat = AC_SRC_ALPHA;

	splashWindow = hwndSplash;
	// paint the window (in the right location) with the alpha-blended bitmap
	UpdateLayeredWindow(splashWindow, g_hdcScreen, &ptOrigin, &_sizeSplash,
		g_hdcMem, &ptZero, RGB(0, 0, 0), &g_blend, ULW_ALPHA);

	// delete temporary objects
	SelectObject(g_hdcMem, hbmpOld);
	DeleteDC(g_hdcMem);
	ReleaseDC(NULL, g_hdcScreen);

	ShowWindow(splashWindow, _nCmdShow);
	if (setSize) {
		g_sizeSplash = _sizeSplash;
	}
}

IStream * CreateStreamOnResource(LPCTSTR lpName, LPCTSTR lpType) {
	// initialize return value
	IStream * ipStream = NULL;

	// find the resource
	HRSRC hrsrc = FindResource(NULL, lpName, lpType);
	if (hrsrc == NULL)
		return ipStream;

	// load the resource
	DWORD dwResourceSize = SizeofResource(NULL, hrsrc);
	HGLOBAL hglbImage = LoadResource(NULL, hrsrc);
	if (hglbImage == NULL)
		return ipStream;

	// lock the resource, getting a pointer to its data
	LPVOID pvSourceResourceData = LockResource(hglbImage);
	if (pvSourceResourceData == NULL)
		return ipStream;

	// allocate memory to hold the resource data
	HGLOBAL hgblResourceData = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);
	if (hgblResourceData == NULL)
		return ipStream;

	// get a pointer to the allocated memory
	LPVOID pvResourceData = GlobalLock(hgblResourceData);
	if (pvResourceData == NULL) {
		GlobalFree(hgblResourceData);
		return ipStream;
	}

	// copy the data from the resource to the new memory block
	CopyMemory(pvResourceData, pvSourceResourceData, dwResourceSize);
	GlobalUnlock(hgblResourceData);

	// create a stream on the HGLOBAL containing the data
	if (SUCCEEDED(CreateStreamOnHGlobal(hgblResourceData, TRUE, &ipStream)))
		return ipStream;
}

IWICBitmapSource * LoadBitmapFromStream(IStream * ipImageStream) {
	// initialize return value
	IWICBitmapSource * ipBitmap = NULL;

	// load WIC's PNG decoder
	IWICBitmapDecoder * ipDecoder = NULL;
	if (FAILED(CoCreateInstance(CLSID_WICPngDecoder, NULL, CLSCTX_INPROC_SERVER, __uuidof(ipDecoder), reinterpret_cast<void**>(&ipDecoder))))
		return ipBitmap;

	// load the PNG
	if (FAILED(ipDecoder->Initialize(ipImageStream, WICDecodeMetadataCacheOnLoad))) {
		ipDecoder->Release();
		return ipBitmap;
	}

	// check for the presence of the first frame in the bitmap
	UINT nFrameCount = 0;
	if (FAILED(ipDecoder->GetFrameCount(&nFrameCount)) || nFrameCount != 1) {
		ipDecoder->Release();
		return ipBitmap;
	}

	// load the first frame (i.e., the image)
	IWICBitmapFrameDecode * ipFrame = NULL;
	if (FAILED(ipDecoder->GetFrame(0, &ipFrame))) {
		ipDecoder->Release();
		return ipBitmap;
	}

	// convert the image to 32bpp BGRA format with pre-multiplied alpha
	//   (it may not be stored in that format natively in the PNG resource,
	//   but we need this format to create the DIB to use on-screen)
	WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, ipFrame, &ipBitmap);
	ipFrame->Release();
	ipDecoder->Release();
	return ipBitmap;
}

HBITMAP CreateHBITMAP(IWICBitmapSource * ipBitmap) {
	// initialize return value
	HBITMAP hbmp = NULL;

	// get image attributes and check for valid image
	UINT width = 0;
	UINT height = 0;
	if (FAILED(ipBitmap->GetSize(&width, &height)) || width == 0 || height == 0)
		return hbmp;

	// prepare structure giving bitmap information (negative height indicates a top-down DIB)
	BITMAPINFO bminfo;
	ZeroMemory(&bminfo, sizeof(bminfo));
	bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bminfo.bmiHeader.biWidth = width;
	bminfo.bmiHeader.biHeight = -((LONG)height);
	bminfo.bmiHeader.biPlanes = 1;
	bminfo.bmiHeader.biBitCount = 32;
	bminfo.bmiHeader.biCompression = BI_RGB;

	// create a DIB section that can hold the image
	void * pvImageBits = NULL;
	HDC hdcScreen = GetDC(NULL);
	hbmp = CreateDIBSection(hdcScreen, &bminfo, DIB_RGB_COLORS, &pvImageBits, NULL, 0);
	ReleaseDC(NULL, hdcScreen);
	if (hbmp == NULL)
		return hbmp;

	// extract the image into the HBITMAP
	const UINT cbStride = width * 4;
	const UINT cbImage = cbStride * height;
	if (FAILED(ipBitmap->CopyPixels(NULL, cbStride, cbImage, static_cast<BYTE *>(pvImageBits))))
	{
		// couldn't extract image; delete HBITMAP
		DeleteObject(hbmp);
		hbmp = NULL;
	}
	return hbmp;
}

HBITMAP LoadSplashImage(int resId) {
	HBITMAP hbmpSplash = NULL;

	// load the PNG image data into a stream
	IStream * ipImageStream = CreateStreamOnResource(MAKEINTRESOURCE(resId), _T("PNG"));
	if (ipImageStream == NULL)
		return hbmpSplash;

	// load the bitmap with WIC
	IWICBitmapSource * ipBitmap = LoadBitmapFromStream(ipImageStream);
	if (ipBitmap == NULL) {
		ipImageStream->Release();
		return hbmpSplash;
	}

	// create a HBITMAP containing the image
	hbmpSplash = CreateHBITMAP(ipBitmap);
	ipBitmap->Release();
	ipImageStream->Release();
	return hbmpSplash;
}

HWND CreateSplashWindow(HINSTANCE g_hInstance) {
	HWND hwndOwner = CreateWindow(szWindowClass, NULL, WS_POPUP,
		0, 0, 0, 0, NULL, NULL, g_hInstance, NULL);
	return CreateWindowEx(WS_EX_LAYERED, szWindowClass, NULL, WS_POPUP | WS_VISIBLE,
		0, 0, 0, 0, hwndOwner, NULL, g_hInstance, NULL);
}