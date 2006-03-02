////////////////////////////////////////////////////////////////////////////////
// Plainamp Rebar Vis Plugin
// 
// Copyright © 2006  Sebastian Pipping <webmaster@hartwork.org>
// 
// -->  http://www.hartwork.org
// 
// This source code is released under the GNU General Public License (GPL).
// See GPL.txt for details. Any non-GPL usage is strictly forbidden.
////////////////////////////////////////////////////////////////////////////////


#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#include "../Winamp/vis.h"
#include "../Winamp/wa_ipc.h"



#define PLUGIN_NAME     "Plainamp Rebar Vis Plugin"
#define PLUGIN_VERSION  "0.2"

#define PLUGIN_DESC     PLUGIN_NAME " " PLUGIN_VERSION



winampVisModule * getModule( int which );


void config( struct winampVisModule * this_mod );
int init( struct winampVisModule * this_mod );
int render_spec( struct winampVisModule * this_mod );
void quit( struct winampVisModule * this_mod );


// Double buffering data
HDC memDC = NULL;      // Memory device context
HBITMAP	memBM = NULL;  // Memory bitmap (for memDC)
HBITMAP	oldBM = NULL;  // Old bitmap (from memDC)


HWND hRenderTarget = NULL;
int width = 0;  
int height = 0;  
bool bRunning = false;
HPEN pen = NULL;


WNDPROC WndprocTargetBackup = NULL;
LRESULT CALLBACK WndprocTarget( HWND hwnd, UINT message, WPARAM wp, LPARAM lp );



winampVisHeader hdr = {
	VIS_HDRVER,
	PLUGIN_DESC,
	getModule
};



winampVisModule mod_spec =
{
	"Spectrum Analyser",
	NULL,	// hwndParent
	NULL,	// hDllInstance
	0,		// sRate
	0,		// nCh
	25,		// latencyMS
	25,		// delayMS
	2,		// spectrumNch
	0,		// waveformNch
	{ 0, },	// spectrumData
	{ 0, },	// waveformData
	config,
	init,
	render_spec, 
	quit
};



#ifdef __cplusplus
extern "C" {
#endif
__declspec( dllexport ) winampVisHeader * winampVisGetHeader()
{
	return &hdr;
}
#ifdef __cplusplus
}
#endif



winampVisModule * getModule( int which )
{
	return which ? NULL : &mod_spec;
}



void config( struct winampVisModule * this_mod )
{
	MessageBox(
		this_mod->hwndParent,
		PLUGIN_DESC "\n"
		"\n"
		"Copyright © 2006 Sebastian Pipping  \n"
		"<webmaster@hartwork.org>\n"
		"\n"
		"-->  http://www.hartwork.org",
		"About",
		MB_ICONINFORMATION
	);
}



void ResizeContext( HWND h, int cx, int cy )
{
	width  = cx;
	height = cy;

	
	HDC memDC_COPY = memDC;
	HBITMAP	memBM_COPY = memBM;
	
	const HDC hdc = GetDC( h );
		memDC = CreateCompatibleDC( hdc );
		memBM = CreateCompatibleBitmap( hdc, width, height );
		oldBM = ( HBITMAP )SelectObject( memDC, memBM );
	ReleaseDC( h, hdc );

	if( memDC_COPY != NULL )
	{
		DeleteObject( memDC_COPY );
		DeleteObject( memBM_COPY );
	}
}



LRESULT CALLBACK WndprocTarget( HWND hwnd, UINT message, WPARAM wp, LPARAM lp )
{
	switch( message )
	{
/*
	case WM_SHOWWINDOW:
		if( wp == FALSE )
		{
			bRunning = false;
		}
		break;
*/
	case WM_SIZE:
		ResizeContext( hwnd, LOWORD( lp ), HIWORD( lp ) );
		break;
		
	}
	return CallWindowProc( WndprocTargetBackup, hwnd, message, wp, lp );
}



int init( struct winampVisModule * this_mod )
{
	if( !this_mod ) return 1;

	// Register message
	const int IPC_GETPLAINBARTARGET = ( int )SendMessage( this_mod->hwndParent, WM_WA_IPC, ( WPARAM )"IPC_GETPLAINBARTARGET", IPC_REGISTER_WINAMP_IPCMESSAGE );
	if( IPC_GETPLAINBARTARGET <= 0 ) return 1;
	
	// Get render target
	hRenderTarget = ( HWND )SendMessage( this_mod->hwndParent, WM_WA_IPC, 0, IPC_GETPLAINBARTARGET );
	if( !IsWindow( hRenderTarget ) ) return 1;

	// Exchange window procedure
	WndprocTargetBackup = ( WNDPROC )GetWindowLong( hRenderTarget, GWL_WNDPROC );
	if( WndprocTargetBackup != NULL )
	{
		SetWindowLong( hRenderTarget, GWL_WNDPROC, ( LONG )WndprocTarget );
	}
	
	RECT r;
	GetClientRect( hRenderTarget, &r );
	width = r.right - r.left;
	height = r.bottom - r.top;
	
	// Create doublebuffer
	const HDC hdc = GetDC( hRenderTarget );
		memDC = CreateCompatibleDC( hdc );
		memBM = CreateCompatibleBitmap( hdc, width, height );
		oldBM = ( HBITMAP )SelectObject( memDC, memBM );
	ReleaseDC( hRenderTarget, hdc );

	pen = CreatePen( PS_SOLID, 0, GetSysColor( COLOR_APPWORKSPACE ) );

	bRunning = true;
	
	return 0;
}



int render_spec( struct winampVisModule * this_mod )
{
	// Clear background
	RECT rect = { 0, 0, width, height };
	FillRect(memDC, &rect, ( HBRUSH )( COLOR_3DFACE + 1 ) );
	
	// Draw analyser
	SelectObject( memDC, pen );
	for( int x = 0; x < width; x++ )
	{
		int val = 0;
		
		const int ix = x * 576 / width;
		for( int y = 0; y < this_mod->nCh; y++ )
		{
			if( this_mod->spectrumData[ y ][ ix ] > val )
			{
				val = this_mod->spectrumData[ y ][ ix ];
			}
		}
		
		MoveToEx( memDC, x, height, NULL );
		LineTo( memDC, x, ( 256 - val ) * height / 256 );
	}

	// Copy doublebuffer to window
	HDC hdc = GetDC( hRenderTarget );
	BitBlt( hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY );
	ReleaseDC( hRenderTarget, hdc );

	return bRunning ? 0 : 1;
}



void quit( struct winampVisModule * this_mod )
{
	bRunning = false;
	
	// Delete doublebuffer
	SelectObject( memDC, oldBM );
	DeleteObject( memDC );
	DeleteObject( memBM );
	
	DeleteObject( pen );
}
