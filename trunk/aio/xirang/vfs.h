#ifndef AIO_XIRANG_VFS_H
#define AIO_XIRANG_VFS_H

#include <aio/xirang/xrbase.h>
#include <aio/common/io.h>
#include <aio/common/fsutility.h>
#include <aio/common/mpl.h>

// STD
#include <memory>

namespace xirang { namespace io{
	template<typename T> struct interface_mask;

	template<> struct interface_mask<aio::io::sequence>{
		static const unsigned long long value = 1;
	};
	template<> struct interface_mask<aio::io::forward>{
		static const unsigned long long value = 1 << 1;
	};
	template<> struct interface_mask<aio::io::random>{
		static const unsigned long long value = 1 << 2;
	};
	template<> struct interface_mask<aio::io::reader>{
		static const unsigned long long value = 1 << 3;
	};
	template<> struct interface_mask<aio::io::writer>{
		static const unsigned long long value = 1 << 4;
	};
	template<> struct interface_mask<aio::io::ioctrl>{
		static const unsigned long long value = 1 << 5;
	};
	template<> struct interface_mask<aio::io::options>{
		static const unsigned long long value = 1 << 6;
	};
	template<> struct interface_mask<aio::io::read_view>{
		static const unsigned long long value = 1 << 7;
	};
	template<> struct interface_mask<aio::io::write_view>{
		static const unsigned long long value = 1 << 8;
	};
	template<> struct interface_mask<aio::io::read_map>{
		static const unsigned long long value = 1 << 9;
	};
	template<> struct interface_mask<aio::io::write_map>{
		static const unsigned long long value = 1 << 10;
	};
	
	template<typename I, typename... Interfaces> struct get_mask{
		static const unsigned long long value = interface_mask<I>::value | get_mask<Interfaces...>::value;
	};
	template<typename T> struct get_mask<T>{
		static const unsigned long long value = interface_mask<T>::value;
	};
}

namespace fs 
{ 
	AIO_EXCEPTION_TYPE(unsupport_interface);

	namespace private_{
		template<typename... Interfaces> struct map_to_iref;
		template<typename... Interfaces> struct map_to_iref<aio::mpl::seq<Interfaces...>>{
			typedef aio::iref<Interfaces...> type;
		};

		template<typename T, typename U> struct less_interface{
			static const bool value = io::interface_mask<T>::value < io::interface_mask<U>::value;
		};

		template<typename... Interfaces> struct sorted_iref{
			typedef aio::mpl::seq<Interfaces...> seq;
			typedef typename aio::mpl::sort<seq, less_interface>::type  sorted_seq;
			typedef typename map_to_iref<sorted_seq>::type type;
		};

		template<typename... Interfaces> struct map_to_iauto;
		template<typename... Interfaces> struct map_to_iauto<aio::mpl::seq<Interfaces...>>{
			typedef aio::iauto<Interfaces...> type;
		};

		template<typename... Interfaces> struct sorted_iauto{
			typedef aio::mpl::seq<Interfaces...> seq;
			typedef typename aio::mpl::sort<seq, less_interface>::type  sorted_seq;
			typedef typename map_to_iauto<sorted_seq>::type type;
		};

		template<typename... Interfaces> struct copy_interface_helper;
		
		template<> struct copy_interface_helper<aio::mpl::seq<>> {
			template<typename IRef>
			static void** copy(unsigned long long , void** ret, const IRef& , void* ){ 
				return ret;
			}
		};
		template<typename T, typename... Interfaces> 
			struct copy_interface_helper<aio::mpl::seq<T, Interfaces...>> {
			template<typename IRef>
			static void** copy(unsigned long long mask, void** ret, const IRef& ref, void* this_){
				if (mask & io::interface_mask<T>::value) {
					*ret++ = *(void**)&ref.template get<T>();
					*ret++ = this_;
				}
				return copy_interface_helper<aio::mpl::seq<Interfaces...> >::template copy(mask, ret, ref, this_);
			}
		};
	}
	template<typename... Interfaces> struct copy_interface{
		template<typename CoClass> 
			static void** apply(unsigned long long mask, void** ret, CoClass& ref, void* this_){
				if((mask & io::get_mask<Interfaces...>::value) != mask)
					AIO_THROW(unsupport_interface);

				typedef typename private_::sorted_iref<Interfaces...>::sorted_seq seq;

				return private_::copy_interface_helper<seq>::template copy(mask, ret, ref, this_);
			}

	};


	class IVfs;
	class RootFs;
	class RootFsImp;

    using aio::fs::fs_error;
    using aio::fs::file_state;
    using aio::any;

    namespace aiofs = aio::fs;

	struct VfsNode
	{
		string path;
		IVfs* owner_fs;
	};

	struct MountInfo
	{
		string path;
		IVfs* fs;
	};

	struct VfsState
	{
		VfsNode node;
		file_state state;
		aio::long_size_t  size;
	};
    enum vfs_option
    {
        vo_default = 0,
        vo_readonly,
        vo_sync_on_destroy,
        vo_user = 1 << 16
    };

	typedef BiRangeT<aio::const_itr_traits<MountInfo> > VfsRange;
	typedef BiRangeT<aio::const_itr_traits<VfsNode> > VfsNodeRange;
	template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type 
		create(IVfs& fs, const aio::const_range_string& path, int flag);

    /// \notes 1. all vfs modifications will return er_ok or not null archive_ptr if succeeded.
    ///     2. the path or file name must be aio style, separated by "/".
    ///     3. supported file name character set is depends on vfs implementation.
    ///     4. the returned fs_error is implementation defined, if user just depends on IVfs, should not assume 
    ///         which error code will be returned, except return er_ok case. but, for a known vfs imp, the error code should be determinated.
    /// 
	class IVfs
	{
		public:
		/// common operations of dir and file
        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \notes the result of removeing a opended file is depends on implementation capability.. 
		virtual fs_error remove(const string& path) = 0;

		/// create dir
        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \notes if the parent of path is not exist, failed.
        /// \notes the result depends on implementation capability.
		virtual fs_error createDir(const  string& path) = 0;

		template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type 
			create(const aio::const_range_string& path, int flag){
			return fs::create<Interfaces...>(*this, path, flag);
		}

        /// copy file via file path
        /// \pre path must not end with '/'. 
		/// \pre !absolute(to)
        /// \pre !absolute(from) || mounted()
		/// if from and to in same fs, it may have a more effective implementation
        /// \notes copy to self is imp defined.
		virtual fs_error copy(const string& from, const string& to) = 0;

        /// truncate a file with given size
        /// \pre path must not end with '/'. 
		/// \pre !absolute(to)
		virtual fs_error truncate(const string& path, aio::long_size_t s) = 0;

        /// flush buffered data to underly media. implementation defined.
		virtual void sync() = 0;
		
		/// query
		virtual const string& resource() const = 0;

		/// volume
		/// if !mounted, return null
		virtual RootFs* root() const = 0;

		/// \post mounted() && root() || !mounted() && !root()
		virtual bool mounted() const = 0;

		/// \return mounted() ? absolute() : empty() 
		virtual string mountPoint() const = 0;

        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \return returned path in VfsNode is file name, not full path.
		virtual VfsNodeRange children(const string& path) const = 0;

        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \return returned path in VfsNode is full path. it's different from children()
		virtual VfsState state(const string& path) const = 0;
	
        /// get the option with given id and data
        /// if failed, return a empty any, otherwise depends on the input value.
        /// all options are implementation defined
        virtual any getopt(int id, const any & optdata = any() ) const ;

        /// set the option with given id, option data
        /// if failed, return a empty any, otherwise depends the input value.
        /// all options are implementation defined
        virtual any setopt(int id, const any & optdata,  const any & indata= any());

		virtual ~IVfs();

		/// create or open file
        /// \param compound flag of aio::io::archive_mode
        /// \param flag one of aio::io::open_flag
        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \notes the capability of returned archive_ptr must greater than or equal to given mode. 
        ///         but if user code just depends on IVfs, should not assume the addational capability not specified by given mode
        /// \notes if the parent of path is not exist, failed.
        /// \notes the result depends on implementation capability.
		virtual void** do_create(unsigned long long mask,
				void* ret, aio::unique_ptr<void>& owner, const string& path, int flag) = 0;

	private:
		// if r == null, means unmount, used by RootFs only
		virtual void setRoot(RootFs* r) = 0;

		friend class RootFsImp;
	};
	template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type 
		create(IVfs& fs, const aio::range_string& path, int flag){
			typedef typename private_::sorted_iauto<Interfaces...>::type interface_type;
			const auto mask = io::get_mask<Interfaces...>::value;
			interface_type ret;
			aio::target_holder<void>* holder = &ret;
			void** last = fs.do_create(mask, (void**)(holder + 1), ret.target_ptr, path, flag);
			AIO_PRE_CONDITION(last == (void**)&ret.target_ptr);
			holder->target = ret.target_ptr.get();
			return std::move(ret);
		}

	template<typename CoClass> struct IVfsCoBase : public IVfs
	{
		virtual fs_error remove(const string& path) {
			return aio::get_cobj<CoClass>(this).remove(path);
		}

		virtual fs_error createDir(const  string& path){
			return aio::get_cobj<CoClass>(this).createDir(path);
		}

		virtual fs_error copy(const string& from, const string& to){
			return aio::get_cobj<CoClass>(this).copy(from, to);
		}

		virtual fs_error truncate(const string& path, aio::long_size_t s){
			return aio::get_cobj<CoClass>(this).truncate(path, s);
		}

		virtual void sync(){
			return aio::get_cobj<CoClass>(this).sync();
		}
		
		virtual const string& resource() const{
			return aio::get_cobj<CoClass>(this).resource();
		}

		virtual RootFs* root() const{
			return aio::get_cobj<CoClass>(this).root();
		}

		virtual bool mounted() const{
			return aio::get_cobj<CoClass>(this).mounted();
		}

		virtual string mountPoint() const{
			return aio::get_cobj<CoClass>(this).mountPoint();
		}

		virtual VfsNodeRange children(const string& path) const{
			return aio::get_cobj<CoClass>(this).children(path);
		}

		virtual VfsState state(const string& path) const{
			return aio::get_cobj<CoClass>(this).state(path);
		}
	
        virtual any getopt(int id, const any & optdata) const{
			return aio::get_cobj<CoClass>(this).getopt(id, optdata);
		}

        virtual any setopt(int id, const any & optdata, const any & indata){
			return aio::get_cobj<CoClass>(this).setopt(id, optdata, indata);
		}

	private:
		virtual void** do_create(unsigned long long mask,
				void** base, aio::unique_ptr<void>& owner, const string& path, int flag) = 0;

		virtual void setRoot(RootFs* r){
			return aio::get_cobj<CoClass>(this).setRoot(r);
		}
	};

	template<typename CoClass>
	struct IVfsCo : IVfsCoBase<CoClass>{
		virtual void** do_create(unsigned long long mask,
				void** base, aio::unique_ptr<void>& owner, const string& path, int flag){
			return aio::get_cobj<CoClass>(this).do_create(mask, base, owner, path, flag);
		}
	};

	template<typename CoClass>
	IVfsCo<CoClass> get_interface_map(IVfs*, CoClass*);

	class RootFs 
	{
		public:	
			explicit RootFs(const string& res);
			~RootFs();

			// \pre dir must be absolute name
			fs_error mount(const string& dir, IVfs& vfs);
			fs_error unmount(const string& dir);

			void sync();

			// query
			const string& resource() const;

			// volume
			// return absolute name
		string mountPoint(const IVfs& p) const;
		IVfs* mountPoint(const string& p) const;

		VfsRange mountedFS() const;
		VfsState locate(const string& path) const;

        // return true if path contains mount pointer in direct/indirect subdir, excluding self.
		bool containMountPoint(const string& path) const;

	private:
		RootFsImp* m_imp;
	};

	AIO_EXCEPTION_TYPE(PermisionDenied);
	AIO_EXCEPTION_TYPE(FileExist);
	AIO_EXCEPTION_TYPE(FileNotFound);

    using aio::fs::append_tail_slash;
	using aio::fs::normalize;
	using aio::fs::is_normalized;
    using aio::fs::is_filename;
    using aio::fs::dir_filename;
    using aio::fs::ext_filename;

    // return true if p[0] is '/';
	extern bool is_absolute(const string& p);

	template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type 
    temp_file(IVfs& vfs, aio::const_range_string template_ , aio::const_range_string parent_dir
			, int flag = aio::io::of_remove_on_close, string* path = 0){
        AIO_PRE_CONDITION(flag == 0 ||  flag  == aio::io::of_remove_on_close);
        flag |= aio::io::of_create;

        if (vfs.state(parent_dir).state != aio::fs::st_dir)
            AIO_THROW(aio::io::create_failed)("failed to locate the temp directory:")(parent_dir);

        //avoid to make parent start with '/'
        string parent =  parent_dir.empty()? aio::string(parent_dir) : append_tail_slash(parent_dir);

        const int max_try = 10;
        for(int i = 0; i < max_try ; ++i)
        {
            aio::string_builder file_path = parent;
            file_path += aio::fs::private_::gen_temp_name(template_);
			if (vfs.state(string(file_path)).state != aio::fs::st_not_found)
				continue;
			auto ret = create<Interfaces...>(vfs, string(file_path), flag);
			*path = file_path;
			return std::move(ret);
        }

        AIO_THROW(aio::io::create_failed)("failed to create temp file in directory:")(parent_dir);
    }

    string temp_dir(IVfs& vfs, aio::const_range_string template_, aio::const_range_string parent_dir);
    fs_error recursive_remove(IVfs&vfs, const string& path);
    fs_error recursive_create_dir(IVfs&vfs, const string& path);

	template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type 
    recursive_create(IVfs&vfs, const string& path, int flag){
        fs_error err = recursive_create_dir(vfs, dir_filename(path));
        if (err != aio::fs::er_ok && err != aio::fs::er_exist)
			AIO_THROW(aio::io::create_failed)("failed to create the parent directory:")(path);

        return create<Interfaces...>(vfs, path, flag);
	}
}}

#endif //end AIO_XIRANG_VFS_H

