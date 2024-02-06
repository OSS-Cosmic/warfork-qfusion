#include "r_image_buf_manipulator.h"
#include "r_texture_format.h"
#include "stb_ds.h"
#include <bits/stdint-uintn.h>

static void __R_SwapchChannelBlockDispatch8( uint8_t *buf, uint16_t c1, uint16_t c2 )
{
	uint8_t temp = 0;
	uint8_t *channelBlock0 = buf + ( c1 * sizeof( uint8_t ) );
	uint8_t *channelBlock1 = buf + ( c2 * sizeof( uint8_t ) );
	temp = *channelBlock0;
	( *channelBlock0 ) = ( *channelBlock1 );
	( *channelBlock1 ) = temp;
}

static void __R_SwapchChannelBlockDispatch16( uint8_t *buf, uint16_t c1, uint16_t c2 )
{
	uint8_t temp[sizeof( uint16_t )] = { 0 };
	uint8_t *channelBlock0 = buf + ( c1 * sizeof( uint16_t ) );
	uint8_t *channelBlock1 = buf + ( c2 * sizeof( uint16_t ) );
	memcpy( temp, channelBlock0, sizeof( uint16_t ) );
	memcpy( channelBlock0, channelBlock1, sizeof( uint16_t ) );
	memcpy( channelBlock1, temp, sizeof( uint16_t ) );
}

static void __R_SwapchChannelBlockDispatch32( uint8_t *buf, uint16_t c1, uint16_t c2 )
{
	uint8_t temp[sizeof( uint32_t )] = { 0 };
	uint8_t *channelBlock0 = buf + ( c1 * sizeof( uint32_t ) );
	uint8_t *channelBlock1 = buf + ( c2 * sizeof( uint32_t ) );
	memcpy( temp, channelBlock0, sizeof( uint32_t ) );
	memcpy( channelBlock0, channelBlock1, sizeof( uint32_t ) );
	memcpy( channelBlock1, temp, sizeof( uint32_t ) );
}

bool R_SwapChannelInPlace( struct image_buffer_s *target, uint16_t c1, uint16_t c2 )
{
	assert( c1 < R_FormatChannelCount(target->layout.format) );
	assert( c2 < R_FormatChannelCount(target->layout.format)  );
	if( c1 == c2 ) {
		return true;
	}

	void ( *channelBlockDispatch )( uint8_t *block, uint16_t c1, uint16_t c2 );
	switch( target->layout.format) {
		case R_FORMAT_RG8_SNORM:
		case R_FORMAT_BGRA8_UNORM:
		case R_FORMAT_BGR8_UNORM:
		case R_FORMAT_RGB8_UNORM:
		case R_FORMAT_RGBA8_UNORM:
		case R_FORMAT_RG8_UINT:
		case R_FORMAT_RG8_SINT:
		case R_FORMAT_BGRA8_SRGB:
		case R_FORMAT_RGBA8_SNORM:
		case R_FORMAT_RGBA8_UINT:
		case R_FORMAT_RGBA8_SINT:
		case R_FORMAT_RGBA8_SRGB:
			channelBlockDispatch = __R_SwapchChannelBlockDispatch8;
			break;
		case R_FORMAT_RG16_UNORM:
		case R_FORMAT_RG16_SNORM:
		case R_FORMAT_RG16_UINT:
		case R_FORMAT_RG16_SINT:
		case R_FORMAT_RG16_SFLOAT:
		case R_FORMAT_RGBA16_UNORM:
		case R_FORMAT_RGBA16_SNORM:
		case R_FORMAT_RGBA16_UINT:
		case R_FORMAT_RGBA16_SINT:
		case R_FORMAT_RGBA16_SFLOAT:
			channelBlockDispatch = __R_SwapchChannelBlockDispatch16;
			break;
		case R_FORMAT_RG32_UINT:
		case R_FORMAT_RG32_SINT:
		case R_FORMAT_RG32_SFLOAT:
		case R_FORMAT_RGB32_UINT:
		case R_FORMAT_RGB32_SINT:
		case R_FORMAT_RGB32_SFLOAT:
		case R_FORMAT_RGBA32_UINT:
		case R_FORMAT_RGBA32_SINT:
		case R_FORMAT_RGBA32_SFLOAT:
			channelBlockDispatch = __R_SwapchChannelBlockDispatch32;
			break;
		default:
			return false;
	}
	const size_t blockSize = R_FormatBitSizePerBlock( target->layout.format) / 8;
	for( size_t row = 0; row < target->layout.logicalHeight; row++ ) {
		for( size_t column = 0; column < target->layout.logicalHeight; column++ ) {
			uint8_t *const buf = &target->data[target->layout.rowPitch * row + ( column * blockSize )];
			channelBlockDispatch( buf, c1, c2 );
		}
	}
	return true;
}

void R_ResizeImage( const struct image_buffer_s *src, uint16_t scaledWidth, uint16_t scaledHeight, struct image_buffer_s *dest )
{

	const size_t blockSize = R_FormatBitSizePerBlock( src->layout.format ) / 8;
	const uint16_t channelCount = R_FormatChannelCount( src->layout.format);
 
 	struct image_buffer_layout_s updateLayout = {};
	R_CalcImageBufferLayout(scaledWidth, scaledHeight, src->layout.format, src->layout.rowAlignment, &updateLayout);
  R_ConfigureImageBuffer(dest, &updateLayout);

	const float scaleX = ( (float)src->layout.width ) / ( (float)dest->layout.width );
	const float scaleY = ( (float)src->layout.height ) / ( (float)dest->layout.height );

	const float sx2 = scaleX / 2.0f;
	const float sy2 = scaleY / 2.0f;
	float *outputValues = alloca( sizeof( float ) * channelCount );
	float *fetchValues = alloca( sizeof( float ) * channelCount );

	switch( src->layout.format ) {
		case R_FORMAT_RG8_UNORM:
		case R_FORMAT_BGRA8_UNORM:
		case R_FORMAT_BGR8_UNORM:
		case R_FORMAT_RGB8_UNORM:
		case R_FORMAT_RGBA8_UNORM: {
			for( size_t row = 0; row < dest->layout.height; row++ ) {
				for( size_t column = 0; column < dest->layout.width; column++ ) {
					const int srcColumn = (int)( column * scaleX );
					const int srcRow = (int)( row * scaleX );

					uint8_t *const destBlock = &dest->data[dest->layout.rowPitch * row + ( column * blockSize )];
					uint8_t *const srcBlock = &src->data[src->layout.rowPitch * srcRow + ( srcColumn * blockSize )];

					R_DecodeLogicalPixelF( srcBlock, src->layout.format, fetchValues );
					for( size_t c = 0; c < channelCount; c++ ) {
						outputValues[c] = fetchValues[c];
					}

					float numberPoints = 0;
					for( int boxY = (int)srcRow - sx2; boxY < (int)srcRow + sx2; boxY++ ) {
						if( boxY < 0 )
							continue;
						if( boxY > src->layout.height )
							continue;
						for( int boxX = (int)srcRow - sy2; boxX < (int)srcRow + sy2; boxX++ ) {
							if( boxX < 0 )
								continue;
							if( boxX > src->layout.width )
								continue;
							numberPoints++;
							uint8_t *const bboxSrcBlock = &src->data[src->layout.rowPitch * boxY + ( boxX * blockSize )];
							R_DecodeLogicalPixelF( bboxSrcBlock, src->layout.format, fetchValues );
							for( size_t c = 0; c < channelCount; c++ ) {
								outputValues[c] += fetchValues[c];
							}
						}
					}

					for( size_t c = 0; c < channelCount; c++ ) {
						outputValues[c] = outputValues[c] / numberPoints;
					}
					R_EncodeLogicalPixelF( destBlock, dest->layout.format, outputValues );
				}
			}
			break;
		}
		default:
			assert( false );
			break;
	}
}

bool R_FlipTexture( struct image_buffer_s *src, struct image_buffer_s *dest, bool flipx, bool flipy, bool flipdiagonal )
{ 

	struct image_buffer_layout_s updateLayout = { 0 };
	assert( !R_FormatIsCompressed( src->layout.format ) ); // we don't handle compressed blocks this swap logic is swapping whole blocks
	const size_t blockSize = R_FormatBitSizePerBlock( src->layout.format ) / 8;
	if( flipdiagonal ) {
		R_CalcImageBufferLayout( src->layout.height, src->layout.width, src->layout.format, src->layout.rowAlignment, &updateLayout );
		R_ConfigureImageBuffer(dest, &updateLayout);
		for( size_t row = 0; row < src->layout.height; row++ ) {
			const size_t flippedRow = ( flipy ? row : ( src->layout.height - 1 ) - row );
			for( size_t col = 0; col < src->layout.width; col++ ) {
				const size_t flippedColumn = ( flipx ? col : ( src->layout.width - 1 ) - col );
				// diagnaols are transposed so columns are rows and rows are columns
				memcpy( &dest->data[( dest->layout.rowPitch * flippedColumn ) + ( flippedRow * blockSize )], &src->data[( src->layout.rowPitch * row ) + ( col * blockSize )], blockSize );
			}
		}

	} else {
		R_CalcImageBufferLayout( src->layout.width, src->layout.height, src->layout.format, src->layout.rowAlignment, &updateLayout );
		R_ConfigureImageBuffer(dest, &updateLayout);
		for( size_t row = 0; row < src->layout.height; row++ ) {
			const size_t flippedRow = ( flipy ? row : ( src->layout.height - 1 ) - row );
			for( size_t col = 0; col < src->layout.width; col++ ) {
				const size_t flippedColumn = ( flipx ? col : ( src->layout.width - 1 ) - col );
				memcpy( &dest->data[( dest->layout.rowPitch * flippedRow ) + ( flippedColumn * blockSize )], &src->data[( src->layout.rowPitch * row ) + ( col * blockSize )], blockSize );
			}
		}
	}
	return true;
}

bool R_MipMapQuarterInPlace( struct image_buffer_s *src )
{
	assert( !R_FormatIsCompressed( src->layout.format ) ); // won't correctly quarter a compressed blocked

	struct image_buffer_layout_s updateLayout = { 0 };
	R_CalcImageBufferLayout( src->layout.width, src->layout.height, src->layout.format, src->layout.rowAlignment, &updateLayout );

	const size_t blockSize = R_FormatBitSizePerBlock( src->layout.format ) / 8;
	uint16_t channelCount = R_FormatChannelCount( src->layout.format );
	float *c1 = alloca( sizeof( float ) * channelCount );
	float *c2 = alloca( sizeof( float ) * channelCount );
	float *c3 = alloca( sizeof( float ) * channelCount );
	float *c4 = alloca( sizeof( float ) * channelCount );
	float *avgValues = alloca( sizeof( float ) * channelCount );

	uint32_t halfWidth = ( src->layout.width >> 1 );
	uint32_t halfHeight = ( src->layout.height >> 1 );

	for( size_t row = 0; row < halfHeight; row++ ) {
		for( size_t column = 0; column < halfWidth; column++ ) {
			uint8_t *const b1 = &src->data[src->layout.rowPitch * ( row * 2 + 0 ) + ( ( column * 2 + 0 ) * blockSize )];
			uint8_t *const b2 = &src->data[src->layout.rowPitch * ( row * 2 + 1 ) + ( ( column * 2 + 0 ) * blockSize )];
			uint8_t *const b3 = &src->data[src->layout.rowPitch * ( row * 2 + 0 ) + ( ( column * 2 + 1 ) * blockSize )];
			uint8_t *const b4 = &src->data[src->layout.rowPitch * ( row * 2 + 1 ) + ( ( column * 2 + 1 ) * blockSize )];

			uint8_t *const dest = &src->data[updateLayout.rowPitch * row + ( column * blockSize )];

			R_DecodeLogicalPixelF( b1, src->layout.format, c1 );
			R_DecodeLogicalPixelF( b2, src->layout.format, c2 );
			R_DecodeLogicalPixelF( b3, src->layout.format, c3 );
			R_DecodeLogicalPixelF( b4, src->layout.format, c4 );
			for( size_t i = 0; i < channelCount; i++ ) {
				avgValues[i] = ( c1[i] + c2[i] + c3[i] + c4[i] ) / 4.0f;
			}
			R_EncodeLogicalPixelF( dest, src->layout.format, avgValues );
		}
	}

	R_ConfigureImageBuffer( src, &updateLayout ); // we configure the
	return true;
}
