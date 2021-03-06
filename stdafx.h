#pragma once
#include "targetver.h"

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <vector>
#include <array>

#include <Windows.h>
#include <d3d11_1.h>
#include <atlbase.h>
#include <atlstr.h>

CString ErrMsg( HRESULT hr );

#define CHECK(hr) { const HRESULT __hr = ( hr ); if( FAILED( __hr ) ) { printf( "Failed @ %s:%i: %S\n", __FILE__, __LINE__, ErrMsg( __hr ).operator LPCTSTR() ); } }