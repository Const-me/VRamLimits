#include "stdafx.h"

CComPtr<ID3D11Device> device;
CComPtr<ID3D11DeviceContext> context;
D3D_FEATURE_LEVEL featureLevel;

HRESULT InitD3D()
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	hr = D3D11CreateDevice( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
		D3D11_SDK_VERSION, &device, &featureLevel, &context );
	return hr;
}

HRESULT getDedicatedVramAsReported( uint64_t& dedicatedVram )
{
	if( !device )
		CHECK( HRESULT_FROM_WIN32( PEERDIST_ERROR_NOT_INITIALIZED ) );

	CComQIPtr<IDXGIDevice> dxgi = device.p;
	if( !dxgi )
		CHECK( E_NOINTERFACE );

	CComQIPtr<IDXGIAdapter> adapter;
	dxgi->GetAdapter( &adapter );

	DXGI_ADAPTER_DESC adapterDesc;
	adapter->GetDesc( &adapterDesc );
	dedicatedVram = adapterDesc.DedicatedVideoMemory;

	return S_OK;
}

void printBytes( const char* what, uint64_t cb )
{
	constexpr double mul = 1.0 / ( 1024 * 1024 * 1024 );
	printf( "%s: %"  PRIu64 " bytes, %f GB\n", what, cb, cb * mul );
}

HRESULT testUsableVram()
{
	std::vector<CComPtr<ID3D11Texture2D>> textures;
	const CD3D11_TEXTURE2D_DESC desc{ DXGI_FORMAT_R8G8B8A8_UINT, 2048, 2048 };
	
	int i;
	for( i = 0; true; i++ )
	{
		CComPtr<ID3D11Texture2D> tx;
		HRESULT hr = device->CreateTexture2D( &desc, nullptr, &tx );
		if( FAILED( hr ) )
		{
			printf( "%S\n", ErrMsg( hr ).operator LPCTSTR() );
			break;
		}
		textures.emplace_back( std::move( tx ) );
	}
	uint64_t cb = (uint64_t)i * desc.Width * desc.Height * 4;
	printBytes( "Usable VRAM", cb );
	return S_OK;
}

HRESULT mainImpl()
{
	CHECK( InitD3D() );

	uint64_t dedicatedVram;
	CHECK( getDedicatedVramAsReported( dedicatedVram ) );
	
	printBytes( "Reported VRAM", dedicatedVram );

	CHECK( testUsableVram() );
	return S_OK;
}

int main()
{
	const HRESULT hr = mainImpl();
	printf( "%S\n", ErrMsg( hr ).operator LPCTSTR() );
	return hr;
}