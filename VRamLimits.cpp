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

	CComPtr<IDXGIAdapter> adapter;
	CHECK( dxgi->GetAdapter( &adapter ) );

	DXGI_ADAPTER_DESC adapterDesc;
	CHECK( adapter->GetDesc( &adapterDesc ) );
	dedicatedVram = adapterDesc.DedicatedVideoMemory;

	return S_OK;
}

void printBytes( const char* what, uint64_t cb )
{
	constexpr double mul = 1.0 / ( 1024 * 1024 * 1024 );
	printf( "%s: %"  PRIu64 " bytes, %f GB\n", what, cb, cb * mul );
}

std::array<uint8_t, 4> genRandomColor()
{
	std::array<uint8_t, 4> arr;
	for( int i = 0; i < 4; i++ )
		arr[ i ] = rand() & 0xFF;
	return arr;
}

class TextureData
{
	std::vector<uint8_t> buffer;
	const UINT RowPitch, DepthPitch;

public:
	TextureData( UINT w, UINT h ) :
		RowPitch( w * 4 ),
		DepthPitch( w * h * 4 )
	{
		buffer.resize( DepthPitch );
	}
	void randomColor()
	{
		const std::array<uint8_t, 4> arr = genRandomColor();
		uint8_t *pDest = buffer.data();
		uint8_t * const pDestEnd = pDest + buffer.size();
		for( ; pDest < pDestEnd; pDest += 4 )
		{
			pDest[ 0 ] = arr[ 0 ];
			pDest[ 1 ] = arr[ 1 ];
			pDest[ 2 ] = arr[ 2 ];
			pDest[ 3 ] = arr[ 3 ];
		}
	}
	void upload( ID3D11Texture2D* tx ) const
	{
		context->UpdateSubresource( tx, 0, nullptr, buffer.data(), RowPitch, DepthPitch );
	}
};

HRESULT testUsableVram()
{
	// Create 1x1 staging texture
	CComPtr<ID3D11Texture2D> txStaging;
	{
		const CD3D11_TEXTURE2D_DESC descStaging{ DXGI_FORMAT_R8G8B8A8_UINT, 1, 1, 1, 0, 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ };
		CHECK( device->CreateTexture2D( &descStaging, nullptr, &txStaging ) );
	}

	// Create a vector of textures, fill them with some data
	std::vector<CComPtr<ID3D11Texture2D>> textures;
	const CD3D11_TEXTURE2D_DESC desc{ DXGI_FORMAT_R8G8B8A8_UINT, 2048, 2048 };

	TextureData texData{ desc.Width, desc.Height };
	srand( 0 );

	int i;
	for( i = 0; true; i++ )
	{
		// Create
		CComPtr<ID3D11Texture2D> tx;
		HRESULT hr = device->CreateTexture2D( &desc, nullptr, &tx );
		if( FAILED( hr ) )
		{
			printf( "%S\n", ErrMsg( hr ).operator LPCTSTR() );
			break;
		}

		// Upload
		texData.randomColor();
		texData.upload( tx );

		// Save
		textures.emplace_back( std::move( tx ) );
	}
	uint64_t cb = (uint64_t)i * desc.Width * desc.Height * 4;
	printBytes( "Usable VRAM", cb );

	// Verify the texture data is OK
	bool allOk = true;
	srand( 0 );
	const CD3D11_BOX box{ 1024, 1024, 0, 1025, 1025, 1 };
	for( auto tex : textures )
	{
		context->CopySubresourceRegion( txStaging, 0, 0, 0, 0, tex, 0, &box );
		D3D11_MAPPED_SUBRESOURCE mapped;
		CHECK( context->Map( txStaging, 0, D3D11_MAP_READ, 0, &mapped ) );
		const uint8_t* pbMapped = (const uint8_t*)mapped.pData;

		const auto writtenColor = genRandomColor();
		for( int i = 0; i < 4; i++ )
			allOk = allOk && ( pbMapped[ i ] == writtenColor[ i ] );
		context->Unmap( txStaging, 0 );
	}

	if( allOk )
		printf( "Verified texture data\n" );
	else
		printf( "Error, the texture data is wrong\n" );

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