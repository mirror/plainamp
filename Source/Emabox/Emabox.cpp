////////////////////////////////////////////////////////////////////////////////
// ExtraMessageBox
// 
// Copyright © 2006  Sebastian Pipping <webmaster@hartwork.org>
// 
// -->  http://www.hartwork.org
// 
// This source code is released under the GNU General Public License (GPL).
// See GPL.txt for details. Any non-GPL usage is strictly forbidden.
////////////////////////////////////////////////////////////////////////////////


/*
TODO
* realign/recenter after height change
* tab stop order when adding buttons
* offer extra callback?
* auto click timer (one button after XXX seconds)
* allow several checkboxes? radio buttons?

MB_YESNO
MB_YESNOCANCEL 
--> MB_YESNOALL
--> MB_YESNOCANCELALL
--> MB_DEFBUTTON5
--> IDNOALL
--> IDYESALL
*/


#include "Emabox.h"
#include "EmaboxConfig.h"
#include <map>

using namespace std;



const int SPACE_UNDER_CHECKBOX  = 10;
const int SPACE_EXTRA_BOTTOM    = 4;

const TCHAR * const szNeverAgain      = TEXT( "Do not show again" );
const TCHAR * const szRememberChoice  = TEXT( "Remember my choice" );



enum WhichOne
{
	Normal,
	Extended,
	Indirect
};

struct StructPowerBoxData
{
	int * bCheckState;
	HHOOK hCBT;                   // CBT hook handle
	WNDPROC WndprocMsgBoxBackup;  // Old wndproc
	UINT uType;                   // Message box type
	HWND hCheck;                  // Checkbox handle
};

typedef struct StructPowerBoxData PowerBoxData;



map<DWORD, PowerBoxData *> thread_to_data;



void ScreenToClient( HWND h, RECT * r )
{
	POINT p;
	p.x = r->left;
	p.y = r->top;
	ScreenToClient( h, &p );
	
	RECT after;
	after.left    = p.x;
	after.right   = p.x + r->right - r->left;
	after.top     = p.y;
	after.bottom  = p.y + r->bottom - r->top;
	
	memcpy( r, &after, sizeof( RECT ) );
}



LRESULT CALLBACK WndprocMsgBox( HWND hwnd, UINT message, WPARAM wp, LPARAM lp )
{
	// Find data
	PowerBoxData * data;
	const DWORD dwCurThread = GetCurrentThreadId();
	const map<DWORD, PowerBoxData *>::iterator iter = thread_to_data.find( dwCurThread );
	if( iter == thread_to_data.end() ) return 0;
	data = iter->second;
	
	switch( message )
	{
	case WM_COMMAND:
		if( HIWORD( wp ) == BN_CLICKED )
		{
			if( !data->hCheck || ( ( HWND )lp != data->hCheck ) ) break;
			
			const LRESULT res = SendMessage( ( HWND )lp, BM_GETSTATE, 0, 0 );
			const bool bCheckedAfter = ( ( res & BST_CHECKED ) == 0 );
			
			// Update external variable
			*( data->bCheckState ) = bCheckedAfter ? 1 : 0;
			
			SendMessage( ( HWND )lp, BM_SETCHECK, ( bCheckedAfter ) ? BST_CHECKED : 0, 0 );
		}
		break;
		
	case WM_INITDIALOG:
		{
			// Add checkbox
			if( ( data->uType & MB_CHECKMASC ) != 0 )
			{
				// Get original window dimensions
				RECT rw;
				GetWindowRect( hwnd, &rw );
				const int iWindowWidthBefore   = rw.right - rw.left;
				const int iWindowHeightBefore  = rw.bottom - rw.top;

				RECT rc;
				GetClientRect( hwnd, &rc );
				const int iClientWidthBefore   = rc.right - rc.left;
				const int iClientHeightBefore  = rc.bottom - rc.top;



				// Find handle of the text label
				HWND hText = NULL;
				
				const HWND hFirstStatic = FindWindowEx( hwnd, NULL, TEXT( "STATIC" ), NULL );
				if( !hFirstStatic ) break;
				
				const HWND hSecondStatic = FindWindowEx( hwnd, hFirstStatic, TEXT( "STATIC" ), NULL );
				if( !hSecondStatic )
				{
					// Only one static means no icon.
					// So hFirstStatic must be the text window.
					
					hText = hFirstStatic;
				}
				else
				{
					TCHAR szBuf[ 2 ] = TEXT( "" );
					if( !GetWindowText( hSecondStatic, szBuf, 2 ) ) break;
					
					if( *szBuf != TEXT( '\0' ) )
					{
						// Has text so it must be the label
						hText = hSecondStatic;
					}
					else
					{
						hText = hFirstStatic;
					}
				}
				
				RECT rt;
				GetWindowRect( hText, &rt );
				ScreenToClient( hwnd, &rt );
				
				const int iLabelHeight = rt.bottom - rt.top;



				// Get distance between label and the buttons
				const HWND hAnyButton = FindWindowEx( hwnd, NULL, TEXT( "BUTTON" ), NULL );
				if( !hAnyButton ) break;
				
				RECT rab;
				GetWindowRect( hAnyButton, &rab );
				ScreenToClient( hwnd, &rab );
				
				const int SPACE_OVER_CHECKBOX = rab.top - rt.bottom;

				const TCHAR * const szCheckboxLabel =	( data->uType & MB_CHECKNEVERAGAIN )
														? EMA_TEXT_NEVER_AGAIN
														: EMA_TEXT_REMEMBER_CHOICE;

				// Add checkbox
				data->hCheck = CreateWindow(
					TEXT( "BUTTON" ),
					szCheckboxLabel,
					WS_CHILD |
						WS_VISIBLE |
						WS_TABSTOP |
						BS_VCENTER |
						BS_CHECKBOX,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					hwnd,
					NULL,
					GetModuleHandle( NULL ),
					NULL
				);
				
				
				// Set initial check state
				SendMessage( data->hCheck, BM_SETCHECK, *( data->bCheckState ) ? BST_CHECKED : 0, 0 );
				
				// Apply default font
				const HFONT hNewFont = ( HFONT )GetStockObject( DEFAULT_GUI_FONT );
				SendMessage( data->hCheck, WM_SETFONT, ( WPARAM )hNewFont, ( LPARAM )TRUE );
				
				const HDC hdc = GetDC( data->hCheck );
				SIZE size;
				const HFONT hOldFont = ( HFONT )SelectObject( hdc, GetStockObject( DEFAULT_GUI_FONT ) );
				GetTextExtentPoint32( hdc, szCheckboxLabel, _tcslen( szCheckboxLabel ), &size );
				SelectObject( hdc, hOldFont );
				ReleaseDC( data->hCheck, hdc );
				
				const int cyMenuSize         = GetSystemMetrics( SM_CYMENUSIZE );
				const int cxMenuSize         = GetSystemMetrics( SM_CXMENUSIZE );

				const int iNeverAgainWidth   = cxMenuSize + size.cx + 1;
				const int iNeverAgainHeight  = ( cyMenuSize > size.cy ) ? cyMenuSize : size.cy;
				
				RECT rna;
				GetWindowRect( data->hCheck, &rna );
				ScreenToClient( hwnd, &rna );
				
				MoveWindow(
					data->hCheck,
					( iClientWidthBefore - ( iNeverAgainWidth ) ) / 2,
					rt.top + iLabelHeight + SPACE_OVER_CHECKBOX,
					iNeverAgainWidth,
					iNeverAgainHeight,
					FALSE
				);

				
				
				// Move all buttons down (except the checkbox)
				const int iDistance = iNeverAgainHeight + SPACE_UNDER_CHECKBOX;
				HWND hLastButton = NULL;
				for( ; ; )
				{
					hLastButton = FindWindowEx( hwnd, hLastButton, TEXT( "BUTTON" ), NULL );
					if( !hLastButton ) break;
					if( hLastButton == data->hCheck ) continue;
					
					RECT rb;
					GetWindowRect( hLastButton, &rb );
					ScreenToClient( hwnd, &rb );

					MoveWindow( hLastButton, rb.left, rb.top + iDistance, rb.right - rb.left, rb.bottom - rb.top, FALSE );
				}


				// Enlarge dialog
				MoveWindow( hwnd, rw.left, rw.top, iWindowWidthBefore, iWindowHeightBefore + iDistance + SPACE_EXTRA_BOTTOM, FALSE );
			}
			else
			{
				data->hCheck = NULL;
			}
			

			
			// Modify close button
			switch( data->uType & MB_CLOSEMASK )
			{
			case MB_DISABLECLOSE:
				{
					const HMENU hSysMenu = GetSystemMenu( hwnd, FALSE );
					EnableMenuItem( hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED );
				}
				break;
				
			case MB_NOCLOSE:
				{
					const LONG style = GetWindowLong( hwnd, GWL_STYLE );
					if( ( style & WS_SYSMENU ) == 0 ) break;
					SetWindowLong( hwnd, GWL_STYLE, ( LONG )( style - WS_SYSMENU ) );
				}
				break;
				
			}
		}
		break;

	}
	return CallWindowProc( data->WndprocMsgBoxBackup, hwnd, message, wp, lp );
}	


// bool bFound = false;

//////////////////////////////////////////////////////////////////////////////// 
/// 
//////////////////////////////////////////////////////////////////////////////// 
LRESULT CALLBACK HookprocMsgBox( int code, WPARAM wp, LPARAM lp )
{
	// Get hook handle
	PowerBoxData * data;
	const DWORD dwCurThread = GetCurrentThreadId();
	const map<DWORD, PowerBoxData *>::iterator iter = thread_to_data.find( dwCurThread );
	if( iter != thread_to_data.end() )
	{
		data = iter->second;
	}
	else
	{
		return 0;
	}
	
	if( code == HCBT_CREATEWND )
	{
		// MSDN says WE CANNOT TRUST "CBT_CREATEWND"
		// so we use only the window handle
		// and get the class name using "GetClassName". (-> Q106079)
		HWND hwnd = ( HWND )wp;

		// Check windowclass
		TCHAR szClass[ 7 ] = TEXT( "" );
		GetClassName( hwnd, szClass, 7 );
		if( !_tcscmp( szClass, TEXT( "#32770" ) ) )
		{
/*			
			if( bFound )
			{
				return CallNextHookEx( hCBT, code, wp, lp );
			}
			
			bFound = true;
*/			
			// Exchange window procedure
			data->WndprocMsgBoxBackup = ( WNDPROC )GetWindowLong( hwnd, GWL_WNDPROC );
			if( data->WndprocMsgBoxBackup != NULL )
			{
				SetWindowLong( hwnd, GWL_WNDPROC, ( LONG )WndprocMsgBox );
			}
		}
	}
	return CallNextHookEx( data->hCBT, code, wp, lp );
}



inline int ExtraAllTheSame( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, WORD wLanguageId, LPMSGBOXPARAMS lpMsgBoxParams, int * bCheckRes, WhichOne function )
{
	// Add entry to map
	const DWORD dwCurThread = GetCurrentThreadId();
	PowerBoxData * const data = new PowerBoxData;
	data->bCheckState  = bCheckRes;
	data->uType        = ( function != Indirect ) ? uType : lpMsgBoxParams->dwStyle;
	thread_to_data.insert( pair<DWORD, PowerBoxData *>( dwCurThread, data ) );
	
	// Setup this-thread-only hook
	const HHOOK g_hCBT = SetWindowsHookEx( WH_CBT, &HookprocMsgBox, GetModuleHandle( NULL ), dwCurThread );

	int res;
	switch( function )
	{
	case Normal:
		res = MessageBox( hWnd, lpText, lpCaption, uType );
		break;

	case Extended:
		res = MessageBoxEx( hWnd, lpText, lpCaption, uType, wLanguageId );
		break;

	case Indirect:
		res = MessageBoxIndirect( lpMsgBoxParams );
		break;

	}

	// Remove hook
	if( g_hCBT != NULL ) UnhookWindowsHookEx( g_hCBT );

	// Remove entry from map
	const map<DWORD, PowerBoxData *>::iterator iter = thread_to_data.find( dwCurThread );
	if( iter != thread_to_data.end() )
	{
		thread_to_data.erase( iter );
	}
	delete data;
	
	return res;
}



int EmaBox( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, int * bCheckRes )
{
	// Check extra flags
	if( ( uType & MB_EXTRAMASC ) == 0 )
	{
		// No extra
		return MessageBox( hWnd, lpText, lpCaption, uType );
	}
	
	return ExtraAllTheSame( hWnd, lpText, lpCaption, uType, 0, NULL, bCheckRes, Normal );
}



int EmaBoxEx( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, WORD wLanguageId, int * bCheckRes )
{
	// Check extra flags
	if( ( uType & MB_EXTRAMASC ) == 0 )
	{
		// No extra
		return MessageBoxEx( hWnd, lpText, lpCaption, uType, wLanguageId );
	}
	
	return ExtraAllTheSame( hWnd, lpText, lpCaption, uType, wLanguageId, NULL, bCheckRes, Extended );
}



int EmaBoxIndirect( const LPMSGBOXPARAMS lpMsgBoxParams, int * bCheckRes )
{
	// Check extra flags
	if( ( lpMsgBoxParams->dwStyle & MB_EXTRAMASC ) == 0 )
	{
		// No extra
		return MessageBoxIndirect( lpMsgBoxParams );
	}
	
	return ExtraAllTheSame( NULL, NULL, NULL, 0, 0, lpMsgBoxParams, bCheckRes, Indirect );
}
