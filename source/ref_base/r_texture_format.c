#include "r_texture_format.h"

uint32_t R_FormatChannelCount(enum texture_format_e format) {
  switch(format) {
    case R_FORMAT_R8_UNORM: return 1;
    case R_FORMAT_R8_SNORM: return 1;
    case R_FORMAT_R8_UINT: return 1;
    case R_FORMAT_R8_SINT: return 1;
    case R_FORMAT_RGB8_UNORM: return 3;
    case R_FORMAT_BGRA8_UNORM: return 4;
    case R_FORMAT_BGRA8_SRGB: return 4;
    case R_FORMAT_RG8_UNORM: return 2;
    case R_FORMAT_RG8_SNORM: return 2;
    case R_FORMAT_RG8_UINT: return 2;
    case R_FORMAT_RG8_SINT: return 2;
    case R_FORMAT_R16_UNORM: return 1;
    case R_FORMAT_R16_SNORM: return 1;
    case R_FORMAT_R16_UINT: return 1;
    case R_FORMAT_R16_SINT: return 1;
    case R_FORMAT_R16_SFLOAT: return 1;
    case R_FORMAT_RGBA16_UNORM: return 4;
    case R_FORMAT_RGBA16_SNORM: return 4;
    case R_FORMAT_RGBA16_UINT: return 4;
    case R_FORMAT_RGBA16_SINT: return 4;
    case R_FORMAT_RGBA16_SFLOAT: return 4;
    case R_FORMAT_R32_UINT: return 1;
    case R_FORMAT_R32_SINT: return 1;
    case R_FORMAT_R32_SFLOAT: return 1;
    case R_FORMAT_RG32_UINT: return 2;
    case R_FORMAT_RG32_SINT: return 2;
    case R_FORMAT_RG32_SFLOAT: return 2;
    case R_FORMAT_RGB32_UINT: return 3;
    case R_FORMAT_RGB32_SINT: return 3;
    case R_FORMAT_RGB32_SFLOAT: return 3;
    case R_FORMAT_RGBA32_UINT: return 4;
    case R_FORMAT_RGBA32_SINT: return 4;
    case R_FORMAT_RGBA32_SFLOAT: return 4;
    case R_FORMAT_BGR8_UNORM: return 3;
    default:
      assert(false);
  }
  return 0;
}
uint32_t R_FormatChannelBitWidth( enum texture_format_e format, uint16_t channel ) {
  if(channel == 0) {
    switch(format) {
      case R_FORMAT_R8_UNORM: return 8;
      case R_FORMAT_R8_SNORM: return 8;
      case R_FORMAT_R8_UINT: return 8;
      case R_FORMAT_R8_SINT: return 8;
      case R_FORMAT_RGBA8_UNORM: return 8;
      case R_FORMAT_BGRA8_UNORM: return 8;
      case R_FORMAT_BGRA8_SRGB: return 8;
      case R_FORMAT_RG8_UNORM: return 8;
      case R_FORMAT_RG8_SNORM: return 8;
      case R_FORMAT_RG8_UINT: return 8;
      case R_FORMAT_RG8_SINT: return 8;
      case R_FORMAT_R16_UNORM: return 16;
      case R_FORMAT_R16_SNORM: return 16;
      case R_FORMAT_R16_UINT: return 16;
      case R_FORMAT_R16_SINT: return 16;
      case R_FORMAT_R16_SFLOAT: return 16;
      case R_FORMAT_RGBA16_UNORM: return 16;
      case R_FORMAT_RGBA16_SNORM: return 16;
      case R_FORMAT_RGBA16_UINT: return 16;
      case R_FORMAT_RGBA16_SINT: return 16;
      case R_FORMAT_RGBA16_SFLOAT: return 16;
      case R_FORMAT_R32_UINT: return 32;
      case R_FORMAT_R32_SINT: return 32;
      case R_FORMAT_R32_SFLOAT: return 32;
      case R_FORMAT_BGR8_UNORM: return 8;
      case R_FORMAT_RGB8_UNORM: return 8;
      case R_FORMAT_RG32_UINT: return 32;
      case R_FORMAT_RG32_SINT: return 32;
      case R_FORMAT_RG32_SFLOAT: return 32;
      case R_FORMAT_RGB32_UINT: return 32;
      case R_FORMAT_RGB32_SINT: return 32;
      case R_FORMAT_RGB32_SFLOAT: return 32;
      case R_FORMAT_RGBA32_UINT: return 32;
      case R_FORMAT_RGBA32_SINT: return 32;
      case R_FORMAT_RGBA32_SFLOAT: return 32;
      default:
        assert(false);
    }
  } else if(channel == 1) {

    switch(format) {
      case R_FORMAT_R8_UNORM: return 0;
      case R_FORMAT_R8_SNORM: return 0;
      case R_FORMAT_R8_UINT: return 0;
      case R_FORMAT_R8_SINT: return 0;
      case R_FORMAT_RGBA8_UNORM: return 8;
      case R_FORMAT_BGRA8_UNORM: return 8;
      case R_FORMAT_BGRA8_SRGB: return 8;
      case R_FORMAT_RG8_UNORM: return 8;
      case R_FORMAT_RG8_SNORM: return 8;
      case R_FORMAT_RG8_UINT: return 8;
      case R_FORMAT_RG8_SINT: return 8;
      case R_FORMAT_R16_UNORM: return 0;
      case R_FORMAT_R16_SNORM: return 0;
      case R_FORMAT_R16_UINT: return 0;
      case R_FORMAT_R16_SINT: return 0;
      case R_FORMAT_R16_SFLOAT: return 0;
      case R_FORMAT_RGBA16_UNORM: return 16;
      case R_FORMAT_RGBA16_SNORM: return 16;
      case R_FORMAT_RGBA16_UINT: return 16;
      case R_FORMAT_RGBA16_SINT: return 16;
      case R_FORMAT_RGBA16_SFLOAT: return 16;
      case R_FORMAT_R32_UINT: return 0;
      case R_FORMAT_R32_SINT: return 0;
      case R_FORMAT_R32_SFLOAT: return 0;
      case R_FORMAT_BGR8_UNORM: return 8;
      case R_FORMAT_RGB8_UNORM: return 8;
      case R_FORMAT_RG32_UINT: return 32;
      case R_FORMAT_RG32_SINT: return 32;
      case R_FORMAT_RG32_SFLOAT: return 32;
      case R_FORMAT_RGB32_UINT: return 32;
      case R_FORMAT_RGB32_SINT: return 32;
      case R_FORMAT_RGB32_SFLOAT: return 32;
      case R_FORMAT_RGBA32_UINT: return 32;
      case R_FORMAT_RGBA32_SINT: return 32;
      case R_FORMAT_RGBA32_SFLOAT: return 32;
      default:
        assert(false);
    }
  } else if(channel == 2) {
    switch(format) {
      case R_FORMAT_R8_UNORM: return 0;
      case R_FORMAT_R8_SNORM: return 0;
      case R_FORMAT_R8_UINT: return 0;
      case R_FORMAT_R8_SINT: return 0;
      case R_FORMAT_RGBA8_UNORM: return 8;
      case R_FORMAT_BGRA8_UNORM: return 8;
      case R_FORMAT_BGRA8_SRGB: return 8;
      case R_FORMAT_RG8_UNORM: return 0;
      case R_FORMAT_RG8_SNORM: return 0;
      case R_FORMAT_RG8_UINT: return 0;
      case R_FORMAT_RG8_SINT: return 0;
      case R_FORMAT_R16_UNORM: return 0;
      case R_FORMAT_R16_SNORM: return 0;
      case R_FORMAT_R16_UINT: return 0;
      case R_FORMAT_R16_SINT: return 0;
      case R_FORMAT_R16_SFLOAT: return 0;
      case R_FORMAT_RGBA16_UNORM: return 16;
      case R_FORMAT_RGBA16_SNORM: return 16;
      case R_FORMAT_RGBA16_UINT: return 16;
      case R_FORMAT_RGBA16_SINT: return 16;
      case R_FORMAT_RGBA16_SFLOAT: return 16;
      case R_FORMAT_R32_UINT: return 0;
      case R_FORMAT_R32_SINT: return 0;
      case R_FORMAT_R32_SFLOAT: return 0;
      case R_FORMAT_BGR8_UNORM: return 8;
      case R_FORMAT_RGB8_UNORM: return 8;
      case R_FORMAT_RG32_UINT: return 0;
      case R_FORMAT_RG32_SINT: return 0;
      case R_FORMAT_RG32_SFLOAT: return 0;
      case R_FORMAT_RGB32_UINT: return 32;
      case R_FORMAT_RGB32_SINT: return 32;
      case R_FORMAT_RGB32_SFLOAT: return 32;
      case R_FORMAT_RGBA32_UINT: return 32;
      case R_FORMAT_RGBA32_SINT: return 32;
      case R_FORMAT_RGBA32_SFLOAT: return 32;
      default:
        assert(false);
    }
  } else if(channel == 3) {
    switch(format) {
      case R_FORMAT_R8_UNORM: return 0;
      case R_FORMAT_R8_SNORM: return 0;
      case R_FORMAT_R8_UINT: return 0;
      case R_FORMAT_R8_SINT: return 0;
      case R_FORMAT_RGBA8_UNORM: return 8;
      case R_FORMAT_BGRA8_UNORM: return 8;
      case R_FORMAT_BGRA8_SRGB: return 8;
      case R_FORMAT_RG8_UNORM: return 8;
      case R_FORMAT_RG8_SNORM: return 8;
      case R_FORMAT_RG8_UINT: return 8;
      case R_FORMAT_RG8_SINT: return 8;
      case R_FORMAT_R16_UNORM: return 0;
      case R_FORMAT_R16_SNORM: return 0;
      case R_FORMAT_R16_UINT: return 0;
      case R_FORMAT_R16_SINT: return 0;
      case R_FORMAT_R16_SFLOAT: return 0;
      case R_FORMAT_RGBA16_UNORM: return 16;
      case R_FORMAT_RGBA16_SNORM: return 16;
      case R_FORMAT_RGBA16_UINT: return 16;
      case R_FORMAT_RGBA16_SINT: return 16;
      case R_FORMAT_RGBA16_SFLOAT: return 16;
      case R_FORMAT_R32_UINT: return 0;
      case R_FORMAT_R32_SINT: return 0;
      case R_FORMAT_R32_SFLOAT: return 0;
      case R_FORMAT_BGR8_UNORM: return 0;
      case R_FORMAT_RGB8_UNORM: return 0;
      case R_FORMAT_RG32_UINT: return 0;
      case R_FORMAT_RG32_SINT: return 0;
      case R_FORMAT_RG32_SFLOAT: return 0;
      case R_FORMAT_RGB32_UINT: return 0;
      case R_FORMAT_RGB32_SINT: return 0;
      case R_FORMAT_RGB32_SFLOAT: return 0;
      case R_FORMAT_RGBA32_UINT: return 32;
      case R_FORMAT_RGBA32_SINT: return 32;
      case R_FORMAT_RGBA32_SFLOAT: return 32;
      default:
        assert(false);
    }
  }
  return R_FORMAT_UNKNOWN;
}

bool R_FormatIsCompressed( enum texture_format_e format )
{
	switch( format ) {
		case R_FORMAT_ETC1_R8G8B8_OES: // R_FORMAT_ETC2_R8G8B8_UNORM is a superset
		case R_FORMAT_ETC2_R8G8B8_UNORM:
		case R_FORMAT_ETC2_R8G8B8_SRGB:
		case R_FORMAT_ETC2_R8G8B8A1_UNORM:
		case R_FORMAT_ETC2_R8G8B8A1_SRGB:
		case R_FORMAT_ETC2_R8G8B8A8_UNORM:
		case R_FORMAT_ETC2_R8G8B8A8_SRGB:
		case R_FORMAT_ETC2_EAC_R11_UNORM:
		case R_FORMAT_ETC2_EAC_R11_SNORM:
		case R_FORMAT_ETC2_EAC_R11G11_UNORM:
		case R_FORMAT_ETC2_EAC_R11G11_SNORM:
			return true;
		default:
			break;
	}
	return false;
}

uint32_t R_FormatBlockWidth( enum texture_format_e format ) {
	switch( format ) {
		case R_FORMAT_ETC1_R8G8B8_OES: return 4;
		case R_FORMAT_ETC2_R8G8B8_UNORM:return 4;
		case R_FORMAT_ETC2_R8G8B8_SRGB: return 4;
		case R_FORMAT_ETC2_R8G8B8A1_UNORM: return 4;
		case R_FORMAT_ETC2_R8G8B8A1_SRGB: return 4;
		case R_FORMAT_ETC2_R8G8B8A8_UNORM: return 4;
		case R_FORMAT_ETC2_R8G8B8A8_SRGB: return 4;
		case R_FORMAT_ETC2_EAC_R11_UNORM: return 4;
		case R_FORMAT_ETC2_EAC_R11_SNORM: return 4;
		case R_FORMAT_ETC2_EAC_R11G11_UNORM: return 4;
		case R_FORMAT_ETC2_EAC_R11G11_SNORM: return 4;
		default:
			break;
	}
	return 1;
}

uint32_t R_FormatBlockHeight( enum texture_format_e format )
{
	switch( format ) {
		case R_FORMAT_ETC1_R8G8B8_OES: return 4;
		case R_FORMAT_ETC2_R8G8B8_UNORM:return 4;
		case R_FORMAT_ETC2_R8G8B8_SRGB: return 4;
		case R_FORMAT_ETC2_R8G8B8A1_UNORM: return 4;
		case R_FORMAT_ETC2_R8G8B8A1_SRGB: return 4;
		case R_FORMAT_ETC2_R8G8B8A8_UNORM: return 4;
		case R_FORMAT_ETC2_R8G8B8A8_SRGB: return 4;
		case R_FORMAT_ETC2_EAC_R11_UNORM: return 4;
		case R_FORMAT_ETC2_EAC_R11_SNORM: return 4;
		case R_FORMAT_ETC2_EAC_R11G11_UNORM: return 4;
		case R_FORMAT_ETC2_EAC_R11G11_SNORM: return 4;
		default:
			break;
	}
	return 1;
}

uint32_t R_FormatBitSizePerBlock(enum texture_format_e format) {
  switch(format) {
    case R_FORMAT_R8_UNORM: return 8;
    case R_FORMAT_R8_SNORM: return 8;
    case R_FORMAT_R8_UINT: return 8;
    case R_FORMAT_R8_SINT: return 8;
    case R_FORMAT_RGBA8_UNORM: return 32;
    case R_FORMAT_BGRA8_UNORM: return 32;
    case R_FORMAT_BGRA8_SRGB: return 32;
    case R_FORMAT_RG8_UNORM: return 16;
    case R_FORMAT_RG8_SNORM: return 16;
    case R_FORMAT_RG8_UINT: return 16;
    case R_FORMAT_RG8_SINT: return 16;
    case R_FORMAT_R16_UNORM: return 16;
    case R_FORMAT_R16_SNORM: return 16;
    case R_FORMAT_R16_UINT: return 16;
    case R_FORMAT_R16_SINT: return 16;
    case R_FORMAT_R16_SFLOAT: return 16;
    case R_FORMAT_RGBA16_UNORM: return 64;
    case R_FORMAT_RGBA16_SNORM: return 64;
    case R_FORMAT_RGBA16_UINT: return 64;
    case R_FORMAT_RGBA16_SINT: return 64;
    case R_FORMAT_RGBA16_SFLOAT: return 64;

    case R_FORMAT_R32_UINT: return 32;
    case R_FORMAT_R32_SINT: return 32;
    case R_FORMAT_R32_SFLOAT: return 32;

    case R_FORMAT_BGR8_UNORM: return 24;
    case R_FORMAT_RGB8_UNORM: return 24;

    case R_FORMAT_RG32_UINT: return 64;
    case R_FORMAT_RG32_SINT: return 64;
    case R_FORMAT_RG32_SFLOAT: return 64;
    case R_FORMAT_RGB32_UINT: return 96;
    case R_FORMAT_RGB32_SINT: return 96;
    case R_FORMAT_RGB32_SFLOAT: return 96;
    case R_FORMAT_RGBA32_UINT: return 128;
    case R_FORMAT_RGBA32_SINT: return 128;
    case R_FORMAT_RGBA32_SFLOAT: return 128;
   
    // compressed formats
    case R_FORMAT_ETC1_R8G8B8_OES: return 64;
	  case R_FORMAT_ETC2_R8G8B8_UNORM: return 64;
	  case R_FORMAT_ETC2_R8G8B8_SRGB: return 64;
	  case R_FORMAT_ETC2_R8G8B8A1_UNORM: return 64;
	  case R_FORMAT_ETC2_R8G8B8A1_SRGB: return 64;
	  case R_FORMAT_ETC2_R8G8B8A8_UNORM: return 64;
	  case R_FORMAT_ETC2_R8G8B8A8_SRGB: return 64;
    default:
      assert(false);
  }
  return 0;
}





