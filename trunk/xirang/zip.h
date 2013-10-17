#ifndef XIRANG_ZIP_H_
#define XIRANG_ZIP_H_

#include <xirang/string.h>
#include <xirang/iterator.h>
#include <xirang/io.h>
#include <xirang/path.h>

namespace xirang{ namespace zip{

	struct file_header{
		uint16_t creator_version;
		uint16_t reader_version;
		uint16_t flags;
		uint16_t method;

		uint16_t modified_time;		//MS-DOS time
		uint16_t modified_date;		//MS-DOS data
		uint16_t crc32;

		uint64_t compressed_size;
		uint64_t uncompressed_size;
		file_path name;
		buffer<byte> extra;
		uint32_t external_attrs;
		string comments;
		// private data
		long_offset_t relative_offset;
		io::read_map *package;
	};

	class file_reader;
	class reader;
	class reader_writer;

	typedef bidir_iterator<const_itr_traits<file_header> > package_iterator;

	enum file_type{
		ft_auto,
		ft_raw,
		ft_defalte,
		ft_inflate,
	};
	enum compress_method{
		cm_store = 0,
		cm_deflate = 8,
	};

	AIO_EXCEPTION_TYPE(data_corrupted_exception);

	//support reader, read_map
	// if type() is ft_raw, seek() imp io::random requirement. Otherwise imp forward  requirement.
	//
	class file_reader_imp;
	class file_reader{
		public:
		file_reader();
		~file_reader();
		file_reader(file_reader&& rhs);
		file_reader& operator=(file_reader rhs);
		void swap(file_reader& rhs);

		explicit file_reader(const file_header& fh, file_type type = ft_inflate);

		bool valid() const;
		explicit operator bool() const;
		file_type type() const;

		range<byte*> read(const range<byte*>& buf);
		bool readable() const;
		long_size_t offset() const;
		long_size_t size() const;
		long_size_t seek(long_size_t off);
		iauto<io::read_view> view_rd(ext_heap::handle h);

		file_reader(const file_reader&) = delete;
		file_reader& operator=(const file_reader&) = delete;
		private:
		unique_ptr<file_reader_imp> m_imp;
	};

	class reader_imp;
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

			bool exists(const file_path& name) const;
			const file_header& find(const file_path& name) const;

			range<iterator> items() const;
			range<iterator> items(const file_path& dir) const;

			reader(const reader&) = delete;
			reader& operator=(const reader&) = delete;
		public:
			xirang::unique_ptr<reader_imp> m_imp;
	};

	class reader_writer_imp;
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

			bool exists(const file_path& name) const;
			const file_header& find(const file_path& name) const;

			range<iterator> items() const;
			range<iterator> items(const file_path& dir) const;

			bool append(io::reader& ar, const file_path& name);
			bool append(io::reader& ar, const file_header& h, file_type type);
			bool remove(const file_path& name);
			bool commit();


			reader_writer(const reader_writer&) = delete;
			reader_writer& operator=(const reader_writer&) = delete;
		private:
			unique_ptr<reader_writer_imp> m_imp;
	};

}}

#endif //end XIRANG_ZIP_H_
