#ifndef AIO_XIRANG_VFS_H
#define AIO_XIRANG_VFS_H

#include <xirang/io.h>
#include <xirang/fsutility.h>
#include <xirang/mpl.h>
#include <xirang/type/xrbase.h>

// STD
#include <memory>

namespace xirang { namespace vfs{ namespace detail{
	template<typename T> struct interface_mask;

	template<> struct interface_mask<io::sequence>{
		static const unsigned long long value = 1;
	};
	template<> struct interface_mask<io::forward>{
		static const unsigned long long value = 3;
	};
	template<> struct interface_mask<io::random>{
		static const unsigned long long value = 7;
	};
	template<> struct interface_mask<io::reader>{
		static const unsigned long long value = 1 << 3;
	};
	template<> struct interface_mask<io::writer>{
		static const unsigned long long value = 1 << 4;
	};
	template<> struct interface_mask<io::ioctrl>{
		static const unsigned long long value = 1 << 5;
	};
	template<> struct interface_mask<io::options>{
		static const unsigned long long value = 1 << 6;
	};
	template<> struct interface_mask<io::read_view>{
		static const unsigned long long value = 1 << 7;
	};
	template<> struct interface_mask<io::write_view>{
		static const unsigned long long value = 1 << 8;
	};
	template<> struct interface_mask<io::read_map>{
		static const unsigned long long value = 1 << 9;
	};
	template<> struct interface_mask<io::write_map>{
		static const unsigned long long value = 1 << 10;
	};
	
	template<typename I, typename... Interfaces> struct get_mask{
		static const unsigned long long value = interface_mask<I>::value | get_mask<Interfaces...>::value;
	};
	template<typename T> struct get_mask<T>{
		static const unsigned long long value = interface_mask<T>::value;
	};
}

	AIO_EXCEPTION_TYPE(unsupport_interface);

	namespace private_{
		template<typename... Interfaces> struct map_to_iref;
		template<typename... Interfaces> struct map_to_iref<mpl::seq<Interfaces...>>{
			typedef iref<Interfaces...> type;
		};

		template<typename T, typename U> struct less_interface{
			static const bool value = detail::interface_mask<T>::value < detail::interface_mask<U>::value;
		};

		template<typename... Interfaces> struct sorted_iref{
			typedef mpl::seq<Interfaces...> seq;
			typedef typename mpl::sort<seq, less_interface>::type  sorted_seq;
			typedef typename map_to_iref<sorted_seq>::type type;
		};

		template<typename... Interfaces> struct map_to_iauto;
		template<typename... Interfaces> struct map_to_iauto<mpl::seq<Interfaces...>>{
			typedef iauto<Interfaces...> type;
		};

		template<typename... Interfaces> struct sorted_iauto{
			typedef mpl::seq<Interfaces...> seq;
			typedef typename mpl::sort<seq, less_interface>::type  sorted_seq;
			typedef typename map_to_iauto<sorted_seq>::type type;
		};

		template<typename... Interfaces> struct copy_interface_helper;
		
		template<> struct copy_interface_helper<mpl::seq<>> {
			template<typename IRef>
			static void** copy(unsigned long long , void** ret, const IRef& , void* ){ 
				return ret;
			}
		};
		template<typename T, typename... Interfaces> 
			struct copy_interface_helper<mpl::seq<T, Interfaces...>> {
			template<typename IRef>
			static void** copy(unsigned long long mask, void** ret, const IRef& ref, void* this_){
				if (mask & detail::interface_mask<T>::value) {
					*ret++ = *(void**)&ref.template get<T>();
					*ret++ = this_;
				}
				return copy_interface_helper<mpl::seq<Interfaces...> >::template copy(mask, ret, ref, this_);
			}
		};
	}
	template<typename... Interfaces> struct copy_interface{
		template<typename CoClass> 
			static void** apply(unsigned long long mask, void** ret, CoClass& ref, void* this_){
				if((mask & detail::get_mask<Interfaces...>::value) != mask)
					AIO_THROW(unsupport_interface);

				typedef typename private_::sorted_iref<Interfaces...>::sorted_seq seq;

				return private_::copy_interface_helper<seq>::template copy(mask, ret, ref, this_);
			}

	};


	class IVfs;
	class RootFs;
	class RootFsImp;

    using fs::fs_error;
    using fs::file_state;

	struct VfsNode
	{
		file_path path;
		IVfs* owner_fs;
	};

	struct MountInfo
	{
		file_path path;
		IVfs* fs;
	};

	struct VfsState
	{
		VfsNode node;
		file_state state;
		long_size_t  size;
	};
    enum vfs_option
    {
        vo_default = 0,
        vo_readonly,
        vo_sync_on_destroy,
        vo_user = 1 << 16
    };

	typedef BiRangeT<const_itr_traits<MountInfo> > VfsRange;
	typedef BiRangeT<const_itr_traits<VfsNode> > VfsNodeRange;
	template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type 
		create(IVfs& fs, sub_file_path path, int flag);

    /// \notes 1. all vfs modifications will return er_ok or not null archive_ptr if succeeded.
    ///     2. the path or file name must be xirang style, separated by "/".
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
		virtual fs_error remove(sub_file_path path) = 0;

		/// create dir
        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \notes if the parent of path is not exist, failed.
        /// \notes the result depends on implementation capability.
		virtual fs_error createDir(sub_file_path path) = 0;

		template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type 
			create(sub_file_path path, int flag){
			return xirang::vfs::create<Interfaces...>(*this, path, flag);
		}

        /// copy file via file path
        /// \pre path must not end with '/'. 
		/// \pre !absolute(to)
        /// \pre !absolute(from) || mounted()
		/// if from and to in same fs, it may have a more effective implementation
        /// \notes copy to self is imp defined.
		virtual fs_error copy(sub_file_path from, sub_file_path to) = 0;

        /// truncate a file with given size
        /// \pre path must not end with '/'. 
		/// \pre !absolute(to)
		virtual fs_error truncate(sub_file_path path, long_size_t s) = 0;

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
		virtual file_path mountPoint() const = 0;

        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \return returned path in VfsNode is file name, not full path.
		virtual VfsNodeRange children(sub_file_path path) const = 0;

        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \return returned path in VfsNode is full path. it's different from children()
		virtual VfsState state(sub_file_path path) const = 0;
	
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
        /// \param compound flag of io::archive_mode
        /// \param flag one of io::open_flag
        /// \pre path must not end with '/'. 
		/// \pre !absolute(path)
        /// \notes the capability of returned archive_ptr must greater than or equal to given mode. 
        ///         but if user code just depends on IVfs, should not assume the addational capability not specified by given mode
        /// \notes if the parent of path is not exist, failed.
        /// \notes the result depends on implementation capability.
		virtual void** do_create(unsigned long long mask,
				void** ret, unique_ptr<void>& owner, sub_file_path path, int flag) = 0;

	private:
		// if r == null, means unmount, used by RootFs only
		virtual void setRoot(RootFs* r) = 0;

		friend class RootFsImp;
	};
	template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type 
		create(IVfs& vfs, sub_file_path path, int flag){
			typedef typename private_::sorted_iauto<Interfaces...>::type interface_type;
			const auto mask = detail::get_mask<Interfaces...>::value;
			interface_type ret;
			target_holder<void>* holder = &ret;
			void** last = vfs.do_create(mask, (void**)(holder + 1), ret.target_ptr, path, flag);
			AIO_PRE_CONDITION(last == (void**)&ret.target_ptr);
			holder->target = ret.target_ptr.get();
			return std::move(ret);
		}

	template<typename CoClass> struct IVfsCoBase : public IVfs
	{
		virtual fs_error remove(sub_file_path path) {
			return get_cobj<CoClass>(this).remove(path);
		}

		virtual fs_error createDir(sub_file_path path){
			return get_cobj<CoClass>(this).createDir(path);
		}

		virtual fs_error copy(sub_file_path from, sub_file_path to){
			return get_cobj<CoClass>(this).copy(from, to);
		}

		virtual fs_error truncate(sub_file_path path, long_size_t s){
			return get_cobj<CoClass>(this).truncate(path, s);
		}

		virtual void sync(){
			return get_cobj<CoClass>(this).sync();
		}
		
		virtual const string& resource() const{
			return get_cobj<CoClass>(this).resource();
		}

		virtual RootFs* root() const{
			return get_cobj<CoClass>(this).root();
		}

		virtual bool mounted() const{
			return get_cobj<CoClass>(this).mounted();
		}

		virtual sub_file_path mountPoint() const{
			return get_cobj<CoClass>(this).mountPoint();
		}

		virtual VfsNodeRange children(sub_file_path path) const{
			return get_cobj<CoClass>(this).children(path);
		}

		virtual VfsState state(sub_file_path path) const{
			return get_cobj<CoClass>(this).state(path);
		}
	
        virtual any getopt(int id, const any & optdata) const{
			return get_cobj<CoClass>(this).getopt(id, optdata);
		}

        virtual any setopt(int id, const any & optdata, const any & indata){
			return get_cobj<CoClass>(this).setopt(id, optdata, indata);
		}

	private:
		virtual void** do_create(unsigned long long mask,
				void** base, unique_ptr<void>& owner, sub_file_path path, int flag) = 0;

		virtual void setRoot(RootFs* r){
			return get_cobj<CoClass>(this).setRoot(r);
		}
	};

	template<typename CoClass>
	struct IVfsCo : IVfsCoBase<CoClass>{
		virtual void** do_create(unsigned long long mask,
				void** base, unique_ptr<void>& owner, sub_file_path path, int flag){
			return get_cobj<CoClass>(this).do_create(mask, base, owner, path, flag);
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
		fs_error mount(sub_file_path dir, IVfs& vfs);
		fs_error mount(sub_file_path dir, xirang::unique_ptr<IVfs>& vfs);
		fs_error unmount(sub_file_path dir);

		void sync();

		// query
		const string& resource() const;

		// volume
		// return absolute name
		file_path mountPoint(const IVfs& p) const;
		IVfs* mountPoint(sub_file_path p) const;

		VfsRange mountedFS() const;
		VfsState locate(sub_file_path path) const;

        // return true if path contains mount pointer in direct/indirect subdir, excluding self.
		bool containMountPoint(sub_file_path path) const;
		

		//Methods of IVfs
		fs_error remove(sub_file_path path);
		fs_error createDir(sub_file_path path);
		fs_error copy(sub_file_path from, sub_file_path to) ;
		fs_error truncate(sub_file_path path, long_size_t s) ;
		VfsNodeRange children(sub_file_path path) const ;
		VfsState state(sub_file_path path) const ;
		void** do_create(unsigned long long mask,
				void** ret, unique_ptr<void>& owner, sub_file_path path, int flag);

	template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type 
		create(sub_file_path path, int flag){
			typedef typename private_::sorted_iauto<Interfaces...>::type interface_type;
			const auto mask = detail::get_mask<Interfaces...>::value;
			interface_type ret;
			target_holder<void>* holder = &ret;
			void** last = this->do_create(mask, (void**)(holder + 1), ret.target_ptr, path, flag);
			AIO_PRE_CONDITION(last == (void**)&ret.target_ptr);
			holder->target = ret.target_ptr.get();
			return std::move(ret);
		}
	private:
		RootFsImp* m_imp;
	};

	template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type 
    temp_file(IVfs& vfs, sub_file_path template_ , sub_file_path parent_dir
			, int flag = io::of_remove_on_close, file_path* path = 0){
        AIO_PRE_CONDITION(flag == 0 ||  flag  == io::of_remove_on_close);
        AIO_PRE_CONDITION(!parent_dir.is_root());
        flag |= io::of_create;

        if (vfs.state(parent_dir).state != fs::st_dir)
			AIO_THROW(fs::not_found_exception)("failed to locate the temp directory:")(parent_dir.str());

        const int max_try = 10;
        for(int i = 0; i < max_try ; ++i)
        {
			auto fpath = parent_dir / fs::private_::gen_temp_name(template_);

			if (vfs.state(fpath).state != fs::st_not_found)
				continue;
			auto ret = create<Interfaces...>(vfs, fpath, flag);
			if (path)
				*path = fpath;
			return std::move(ret);
        }

        AIO_THROW(fs::open_failed_exception)("failed to create temp file in directory:")(parent_dir.str());
    }

    file_path temp_dir(IVfs& vfs, sub_file_path template_, sub_file_path parent_dir);
    fs_error recursive_remove(IVfs&vfs, sub_file_path path);
    fs_error recursive_create_dir(IVfs&vfs, sub_file_path path);

	template<typename... Interfaces> typename private_::sorted_iauto<Interfaces...>::type 
    recursive_create(IVfs&vfs, sub_file_path path, int flag){
        fs_error err = recursive_create_dir(vfs, path.parent());
        if (err != fs::er_ok)
			AIO_THROW(fs::open_failed_exception)("failed to create the parent directory:")(path.str());

        return create<Interfaces...>(vfs, path, flag);
	}
}}

#endif //end AIO_XIRANG_VFS_H

