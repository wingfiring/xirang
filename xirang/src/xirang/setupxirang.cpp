#include <aio/xirang/xirang.h>
#include <aio/xirang/typebinder.h>
#include <aio/xirang/nativetype.h>
#include <aio/common/archive/string.h>
#include <aio/xirang/array.h>

#include <assert.h>
namespace xirang
{
	template<> struct serializer<UnitsValue> {
		static aio::archive::writer& apply(aio::archive::writer& wt, ConstCommonObject obj){
			const UnitsValue& var = uncheckBind<UnitsValue>(obj);
			return wt & var.units & var.value;
		}
	};

	template<> struct deserializer<UnitsValue>{
		static aio::archive::reader& apply(aio::archive::reader& rd, CommonObject obj, heap& inner, ext_heap& ext){
			UnitsValue& var = uncheckBind<UnitsValue>(obj);
			return rd & var.units & var.value;
		}
	};

	namespace {
	    template<typename T>
        TypeMethods* getPrimitiveMethods()
        {
            static PrimitiveMethods<T> methods;
            return &methods;
        }
        
		struct TypeDesc
		{
			const char* name;
			TypeMethods* methods;
		};

		TypeDesc g_systype_table[] =
		{
			{"bool", 	getPrimitiveMethods<bool>()},
            {"short", 	getPrimitiveMethods<short>()},
            {"ushort", 	getPrimitiveMethods<unsigned short>()},
            {"int", 	getPrimitiveMethods<int>()},
            {"uint", 	getPrimitiveMethods<unsigned int>()},
            {"long", 	getPrimitiveMethods<long>()},
            {"ulong", 	getPrimitiveMethods<unsigned long>()},
            {"longlong", 	getPrimitiveMethods<long long>()},
            {"ulonglong", 	getPrimitiveMethods<unsigned long long>()},

			{"float",	getPrimitiveMethods<float>()},
			{"double",	getPrimitiveMethods<double>()},

			{"int8", 	getPrimitiveMethods<int8_t>()},
			{"uint8", 	getPrimitiveMethods<uint8_t>()},
			{"int16",	getPrimitiveMethods<int16_t>()},
			{"uint16",	getPrimitiveMethods<uint16_t>()},
			{"int32",	getPrimitiveMethods<int32_t>()},
			{"uint32",	getPrimitiveMethods<uint32_t>()},
			{"int64",	getPrimitiveMethods<int64_t>()},
			{"uint64",	getPrimitiveMethods<uint64_t>()},

			{"byte",	getPrimitiveMethods<unsigned char>()},

			{"pointer",	getPrimitiveMethods<void*>()},
			{"type",	getPrimitiveMethods<Type>()},

			{"string",	getPrimitiveMethods<string>()},

            {"matrix44", getPrimitiveMethods<Matrix44>()},
            {"color3", getPrimitiveMethods<Color3>()},
            {"color4", getPrimitiveMethods<Color4>()},
            {"units_value", getPrimitiveMethods<UnitsValue>()},
            {"vector3", getPrimitiveMethods<Vector3>()},
            {"vector4", getPrimitiveMethods<Vector4>()},
            {"byte_buffer", getPrimitiveMethods<aio::byte_buffer>()},
            {0,0}
		};
	}

    

	typedef aio::basic_range_string<const char> literal;
	void SetupXirang(Xirang& xr)
	{
        Namespace root = xr.root();

        Namespace sys = NamespaceBuilder().name("sys").adoptBy(root);

		for (TypeDesc* itr(g_systype_table); itr->name != 0; ++itr)
		{
            Type t = TypeBuilder(itr->methods)
                .name(itr->name)
                .endBuild()
                .adoptBy(sys);

            TypeAliasBuilder()
                .name(itr->name)
                .typeName(literal(".sys.") + literal(itr->name))
                .setType(t)
                .adoptBy(root);
		}
		
		//static reference type
		{	
            Type t = TypeBuilder()
                .name("ref")
                .setArg("ref_type", "", Type())
                .addMember("to", "pointer", sys.findType("pointer"))
                .endBuild()
                .adoptBy(sys);

            TypeAliasBuilder()
                .name("ref")
                .typeName(".sys.ref")
                .setType(t)
                .adoptBy(root);
		}

		//dynamic reference type
		{	
            Type t = TypeBuilder()
                .name("dynamic_ref")
                .addMember("to", "pointer", sys.findType("pointer"))
                .addMember("type", "type", sys.findType("type"))
                .endBuild()
                .adoptBy(sys);

            TypeAliasBuilder()
                .name("dynamic_ref")
                .typeName(".sys.dynamic_ref")
                .setType(t)
                .adoptBy(root);
		}
		{
            Type t = TypeBuilder(getPrimitiveMethods<Array>())
                .name("array")
                .setArg("value_type", "", Type())
                .endBuild()
                .adoptBy(sys);

            TypeAliasBuilder()
                .name("array")
                .typeName(".sys.array")
                .setType(t)
                .adoptBy(root);
		}
	}
}
