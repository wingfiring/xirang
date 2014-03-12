#ifndef XIRANG_TYPE_NATIVE_TYPE_VERSION_OF_H
#define XIRANG_TYPE_NATIVE_TYPE_VERSION_OF_H
#include <xirang/type/itypebinder.h>
#include <xirang/type/array.h>

#include <xirang/io/buffer.h>
#include <xirang/io/string.h>


namespace xirang{ namespace type{
	XIRANG_DEFINE_TYPE_VERSION_OF("1c7c286d46ab8f436995eeac39079025303422cd",         bool, ".sys.type.bool");
	XIRANG_DEFINE_TYPE_VERSION_OF("c5d52b82e5d9415ce6a3e1e33e6a0741a9f9c841",        float, ".sys.type.float");
	XIRANG_DEFINE_TYPE_VERSION_OF("86caee67a957fc84d807c9ec47e8567c78786fc9",       double, ".sys.type.double");
	XIRANG_DEFINE_TYPE_VERSION_OF("bf2dbf73d1d93cf47c86a7f6f9a35efcba4b1601",       int8_t, ".sys.type.int8");
	XIRANG_DEFINE_TYPE_VERSION_OF("807a3d35a50a37d28b17af1a0517a323d800c8b2",      uint8_t, ".sys.type.uint8");
	XIRANG_DEFINE_TYPE_VERSION_OF("cf29cc8cd4e1a24b5af35537947f970c3a162d8f",      int16_t, ".sys.type.int16");
	XIRANG_DEFINE_TYPE_VERSION_OF("ec3dd6908d495b96ca9fb03e1230044cee95ef50",     uint16_t, ".sys.type.uint16");
	XIRANG_DEFINE_TYPE_VERSION_OF("c78ab2012e4ccfb84e39edc7be677b667ea327b2",      int32_t, ".sys.type.int32");
	XIRANG_DEFINE_TYPE_VERSION_OF("0f5c41cd5fd82dbc353fb761740ddd3a7f5dda68",     uint32_t, ".sys.type.uint32");
	XIRANG_DEFINE_TYPE_VERSION_OF("f026fefdd7e565daf8f835b3a6b6a2df96ea8d1c",      int64_t, ".sys.type.int64");
	XIRANG_DEFINE_TYPE_VERSION_OF("64f3fe94a0573a5058f6b659c957b4265b1b8b01",     uint64_t, ".sys.type.uint64");
	XIRANG_DEFINE_TYPE_VERSION_OF("74a1f3473f939fbf486e2cc661bda65fca34f8fb",         byte, ".sys.type.byte");
	XIRANG_DEFINE_TYPE_VERSION_OF("66dc7f66bc941de38185dacd854cdd26c3d1d84d",        void*, ".sys.type.pointer");
	XIRANG_DEFINE_TYPE_VERSION_OF("7136b6e7cc2d93724f225efbf35987ab9c7a4cf1",       string, ".sys.type.string");
	XIRANG_DEFINE_TYPE_VERSION_OF("d4ee1246b4a8afb6befee004f110cfab950caccf",  byte_buffer, ".sys.type.byte_buffer");
	XIRANG_DEFINE_TYPE_VERSION_OF("374dc6d00c4eec1ff0c95af9943fa55dca4990c0",        Array, ".sys.type.array");
}}
#endif //end XIRANG_TYPE_NATIVE_TYPE_VERSION_OF_H

