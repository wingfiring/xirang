#ifndef AIO_COMMON_ZIP_H_
#define AIO_COMMON_ZIP_H_

#include <xirang/io.h>

namespace xirang{ namespace zip{

	typedef range<const byte*> dict_type;

	///\@see coreponding in zlib
	enum level{
		zl_default = -1,
		zl_no_compression = 0,
		zl_best_speed = 1,
		zl_best_compression = 9
	};

	enum strategy{
		zs_filtered = 1,
		zs_huffman_only = 2,
		zs_default = 0
	};

	enum error{
		ze_ok,
		ze_internal_error,
		ze_data_error,
		ze_mem_error,
		ze_need_dict,
		ze_stream_error,
		ze_unfinished_data,
		ze_crc_error
	};

	struct zip_result{
		int err;
		long_size_t in_size;
		long_size_t out_size;
	};

	/// return crc32 initilaized value
	extern uint32_t crc32_init();

	//caculate new crc from input
	extern uint32_t crc32(io::reader& src, uint32_t crc = crc32_init());
	extern uint32_t crc32(io::read_map& src, uint32_t crc = crc32_init());
	extern uint32_t crc32(range<const byte*>& src, uint32_t crc = crc32_init());

	extern zip_result inflate(io::reader& src, io::writer& dest, dict_type dict = dict_type(),heap* h = 0);
	extern zip_result inflate(io::read_map& src, io::write_map& dest, dict_type dict = dict_type(),  heap* h = 0);

	extern zip_result deflate(io::reader& src, io::writer& dest, int level = zl_default, dict_type dict = dict_type(),heap* h = 0, int strategy_ = zs_default);
	extern zip_result deflate(io::read_map& src, io::write_map& dest, int level = zl_default, dict_type dict = dict_type(),  heap* h = 0, int strategy_ = zs_default);

}}

#endif //end AIO_COMMON_ZIP_H_


