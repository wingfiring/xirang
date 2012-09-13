#ifndef XIRANG_TYPE_BINDER_H
#define XIRANG_TYPE_BINDER_H

#include <aio/xirang/itypebinder.h>

#include <aio/xirang/binder.h>
#include <typeinfo>

namespace xirang
{
	template<typename T>
	struct TypeInfo : TypeInfoHandle
	{
        TypeInfo():TypeInfoHandle(){}
		virtual bool equal(const TypeInfoHandle& other) const 
		{
			return this == &other
				|| typeid(*this) == typeid(other); 
		}
	};

    template<typename T> struct serializer{
        static aio::archive::writer& apply(aio::archive::writer& wt, ConstCommonObject obj){
            return wt & uncheckBind<T>(obj);
        }
    };

    template<typename T> struct deserializer{
        static aio::archive::reader& apply(aio::archive::reader& rd, CommonObject obj, heap& inner, ext_heap& ext){
            return rd & uncheckBind<T>(obj);
        }
    };

    template<typename T> struct assinger{
        static void apply(ConstCommonObject from, CommonObject to){
            uncheckBind<T>(to) = uncheckBind<T>(from);
        }
    };

    template<typename T> struct constructor{
        static void apply(CommonObject obj, heap& /*al */, ext_heap& /*ext*/) {
            new (& uncheckBind<T>(obj)) T();
        }
    };

    template<typename T> struct destructor{
        static void apply(CommonObject obj) {
            (& uncheckBind<T>(obj))->~T();
        }
    };

    template<typename T> struct comparison{
        static int apply(ConstCommonObject lhs,ConstCommonObject rhs) {
            const T& l = *reinterpret_cast<const T*>(lhs.data());
            const T& r = *reinterpret_cast<const T*>(rhs.data());
            return l < r 
                ? -1 : r < l ? 1 : 0;
        }
    };

    template<typename T> struct hasher{
        static size_t apply(ConstCommonObject obj) {
            return (size_t)uncheckBind<T>(obj);
        }
    };

    template<> struct hasher<string>{
        static size_t apply(ConstCommonObject obj);
    };
    
    template<typename T> struct layout{
        static const std::size_t alignment_ = std::alignment_of<T>::value;
        static const bool is_pod_ = std::is_pod<T>::value;
        static const std::size_t size_ = sizeof(T);

        static void begin(std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod)
		{
			payload = size_;
			offset = 0;
			align = alignment_;
			pod = is_pod_;
		}

		static void next(TypeItem& item, std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod)
		{
			AIO_PRE_CONDITION(false && "should no composed item");
		}
    };

    template<typename T> struct extendMethods
    {
        static MethodsExtension* value()
        {
            static MethodsExtension methodsExt = 
            {
                &comparison<T>::apply,
                0//&hasher<T>::apply
            };
            return &methodsExt;
        }
    };

	template<typename T>
	struct PrimitiveMethods : public TypeMethods
	{
		virtual void construct(CommonObject obj, heap&  al , ext_heap& ext) const
		{
			constructor<T>::apply(obj, al, ext);
		}

		virtual void destruct(CommonObject obj) const
		{
			destructor<T>::apply(obj);
		}
		virtual void assign(ConstCommonObject src, CommonObject dest) const{
            assinger<T>::apply(src, dest);
        }

		virtual void deserialize(aio::archive::reader& rd, CommonObject obj, heap& inner, ext_heap& ext) const
		{
			AIO_PRE_CONDITION(obj.valid() );
            deserializer<T>::apply(rd, obj, inner, ext);
		}

		virtual void serialize(aio::archive::writer& wr, ConstCommonObject obj) const
		{
			AIO_PRE_CONDITION(obj.valid() );
            serializer<T>::apply(wr, obj);
		}

        virtual void beginLayout(std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const
		{
            layout<T>::begin(payload, offset, align, pod);
		}

		virtual void nextLayout(TypeItem& item, std::size_t& payload, std::size_t& offset, std::size_t& align, bool& pod) const
		{
			layout<T>::next(item, payload, offset, align, pod);
		}
		
        virtual const TypeInfoHandle& typeinfo() const
		{
			return typeinfo_;
		}

		virtual const MethodsExtension* extension() const
		{
            return extendMethods<T>::value();
		}
		
    private:

		static const TypeInfo<T> typeinfo_;
	};

	template<typename T>
	const TypeInfo<T>	PrimitiveMethods<T>::typeinfo_;

}
#endif //end XIRANG_TYPE_BINDER_H
