#ifndef XIRANG_TYPE_NATIVE_TYPE_VERSION_OF_H
#define XIRANG_TYPE_NATIVE_TYPE_VERSION_OF_H
#include <xirang/type/itypebinder.h>
#include <xirang/type/array.h>

#include <xirang/io/buffer.h>
#include <xirang/io/string.h>


namespace xirang{ namespace type{

	XIRANG_DEFINE_TYPE_VERSION_OF("527bec32fc5758a4d5a845ca12c7a7406f0b60df",         bool, ".sys.type.bool");
	XIRANG_DEFINE_TYPE_VERSION_OF("b922d7de52aa647c063e25dceaded87428c992f0",        float, ".sys.type.float");
	XIRANG_DEFINE_TYPE_VERSION_OF("5b1b6725190e0fddee3568015cee550a5c23c060",       double, ".sys.type.double");
	XIRANG_DEFINE_TYPE_VERSION_OF("625201c858ca07636161d98716135be0266ba5f1",       int8_t, ".sys.type.int8");
	XIRANG_DEFINE_TYPE_VERSION_OF("4cf4551ba205a9060e2b48ae07bac14c66c7611d",      uint8_t, ".sys.type.uint8");
	XIRANG_DEFINE_TYPE_VERSION_OF("75df94e2198bdfdeeb737737f644127c68b2ae80",      int16_t, ".sys.type.int16");
	XIRANG_DEFINE_TYPE_VERSION_OF("c4d3c4b117bbb62056e5e5fa139888715921885c",     uint16_t, ".sys.type.uint16");
	XIRANG_DEFINE_TYPE_VERSION_OF("20f7150f2351a954d6ea2aa2ebd9835891db21e1",      int32_t, ".sys.type.int32");
	XIRANG_DEFINE_TYPE_VERSION_OF("bc1efd808424a2c41dca1d8352bff060f6a3374b",     uint32_t, ".sys.type.uint32");
	XIRANG_DEFINE_TYPE_VERSION_OF("ec9b3fb9fb32eaa54d0f3621f1a39a5c96ace908",      int64_t, ".sys.type.int64");
	XIRANG_DEFINE_TYPE_VERSION_OF("071244bf252378a58d1e9839b72a5a75ec07ab0c",     uint64_t, ".sys.type.uint64");
	XIRANG_DEFINE_TYPE_VERSION_OF("1fdee7fd082d04426d05be099521d4f0781b00bc",         byte, ".sys.type.byte");
	XIRANG_DEFINE_TYPE_VERSION_OF("8127e7595e7d5f2660d15b945cff977720729454",        void*, ".sys.type.pointer");
	XIRANG_DEFINE_TYPE_VERSION_OF("d0aa4cdc5e412330866bb56a1ba8fa18cbf368d4",       string, ".sys.type.string");
	XIRANG_DEFINE_TYPE_VERSION_OF("2aa48e2e8ec84eb9541f0d10ecbaf792c6886408",  byte_buffer, ".sys.type.byte_buffer");
	XIRANG_DEFINE_TYPE_VERSION_OF("c450ebc9dec792d8b4f9a907ba1053ac075de9b4",        Array, ".sys.type.array");

	// HACK: workaround for gcc on Linux
	XIRANG_DEFINE_TYPE_VERSION_OF("ec9b3fb9fb32eaa54d0f3621f1a39a5c96ace908",    long long, ".sys.type.int64");
	XIRANG_DEFINE_TYPE_VERSION_OF("071244bf252378a58d1e9839b72a5a75ec07ab0c", unsigned long long, ".sys.type.uint64");

}}
#endif //end XIRANG_TYPE_NATIVE_TYPE_VERSION_OF_H

