#ifndef XIRANG_ZIP_H_
#define XIRANG_ZIP_H_

#include <xirang/string.h>
#include <xirang/iterator.h>
#include <xirang/io.h>
#include <xirang/path.h>
#include <xirang/deflate.h>

namespace xirang{ namespace zip{

	struct file_header{
		uint16_t creator_version = 20;
		uint16_t reader_version = 20;
		uint16_t flags = 0;
		uint16_t method = 8;

		uint16_t modified_time = 0;		//MS-DOS time
		uint16_t modified_date = 0;		//MS-DOS data
		uint32_t crc32 = 0;

		uint64_t compressed_size = 0;
		uint64_t uncompressed_size = 0;
		file_path name;
		buffer<byte> extra;
		uint32_t external_attrs = 0;
		string comments;
		// private data
		long_size_t relative_offset_ = 0;
		uint16_t local_header_size_ = uint16_t(-1);
		io::read_map* package = 0;
	};

	class reader;
	class reader_writer;

	typedef bidir_iterator<const_itr_traits<file_header> > package_iterator;

	enum file_type{
		ft_raw,
		ft_defalted,
	};
	enum compress_method{
		cm_store = 0,
		cm_deflate = 8,
	};

	AIO_EXCEPTION_TYPE(data_corrupted_exception);

	class zip_package_imp;
	class reader{ 
		public:
			typedef package_iterator const_iterator;
			typedef const_iterator iterator;

			explicit reader(io::read_map& ar);
			reader();
			~reader();
			reader(reader&& rhs);
			reader& operator=(reader&& rhs);
			void swap(reader& rhs);

			bool valid() const;
			explicit operator bool() const;

			bool exists(const file_path& name) const;
			const file_header* get_file(const file_path& name) const;

			range<iterator> items() const;
			range<iterator> items(const file_path& dir) const;

			reader(const reader&) = delete;
			reader& operator=(const reader&) = delete;
		public:
			xirang::unique_ptr<zip_package_imp> m_imp;
	};

	class zip_package_writer_imp;
	class reader_writer{ 
		public:
			typedef package_iterator const_iterator;
			typedef const_iterator iterator;

			reader_writer();
			~reader_writer();
			explicit reader_writer(iref<io::read_map, io::write_map, io::ioctrl> ar);
			reader_writer(reader_writer&& rhs);
			reader_writer& operator=(reader_writer&& rhs);
			void swap(reader_writer& rhs);
			bool valid() const;
			explicit operator bool() const;

			bool exists(const file_path& name) const;
			const file_header* get_file(const file_path& name) const;

			range<iterator> items() const;
			range<iterator> items(const file_path& dir) const;

			const file_header* append(io::reader& ar, const file_path& name, compress_method method = cm_deflate);
			const file_header* append(io::read_map& ar, const file_path& name, compress_method method = cm_deflate);
			const file_header* append(io::reader& ar, const file_header& h, file_type type = ft_raw);
			const file_header* append(io::read_map& ar, const file_header& h, file_type type = ft_raw);

			void sync();

			reader_writer(const reader_writer&) = delete;
			reader_writer& operator=(const reader_writer&) = delete;
		private:
			unique_ptr<zip_package_writer_imp> m_imp;
	};

	extern iauto<io::read_map> open_raw(const file_header& fh);

}}

#endif //end XIRANG_ZIP_H_
