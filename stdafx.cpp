#include "stdafx.h"
#pragma comment(lib, "D3D11.lib")

CString ErrMsg( HRESULT hr )
{
	CString res;
	wchar_t *lpMsgBuf;
	const DWORD fm = FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, 0, (LPTSTR)&lpMsgBuf, 0, NULL );
	if( 0 == fm )
	{
		res.Format( L"Unknown status code = 0x%08x (%i)", hr, hr );
		return res;
	}
	res = lpMsgBuf;
	LocalFree( lpMsgBuf );
	res.Trim( L"\r\n\t " );
	return res;
}