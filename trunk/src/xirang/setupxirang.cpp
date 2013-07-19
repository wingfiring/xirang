#include <aio/xirang/xirang.h>
#include <aio/xirang/typebinder.h>
#include <aio/xirang/nativetype.h>
#include <aio/common/io/string.h>
#include <aio/xirang/array.h>

#include <assert.h>
namespace xirang
{
	namespace {
	    template<typename T>
        TypeMethods* getPrimitiveMethods()
        {
            static PrimitiveMethods<T> methods;
            return &methods;
        }
        
		struct TypeDesc
		{
			aio::const_range_string name;
			TypeMethods* methods;
		};

		using aio::literal;

		TypeDesc g_systype_table[] =
		{
			{literal("bool"), 	getPrimitiveMethods<bool>()},
            {literal("short"), 	getPrimitiveMethods<short>()},
            {literal("ushort"), 	getPrimitiveMethods<unsigned short>()},
            {literal("int"), 	getPrimitiveMethods<int>()},
            {literal("uint"), 	getPrimitiveMethods<unsigned int>()},
            {literal("long"), 	getPrimitiveMethods<long>()},
            {literal("ulong"), 	getPrimitiveMethods<unsigned long>()},
            {literal("longlong"), 	getPrimitiveMethods<long long>()},
            {literal("ulonglong"), 	getPrimitiveMethods<unsigned long long>()},

			{literal("float"),	getPrimitiveMethods<float>()},
			{literal("double"),	getPrimitiveMethods<double>()},

			{literal("int8"), 	getPrimitiveMethods<int8_t>()},
			{literal("uint8"), 	getPrimitiveMethods<uint8_t>()},
			{literal("int16"),	getPrimitiveMethods<int16_t>()},
			{literal("uint16"),	getPrimitiveMethods<uint16_t>()},
			{literal("int32"),	getPrimitiveMethods<int32_t>()},
			{literal("uint32"),	getPrimitiveMethods<uint32_t>()},
			{literal("int64"),	getPrimitiveMethods<int64_t>()},
			{literal("uint64"),	getPrimitiveMethods<uint64_t>()},

			{literal("byte"),	getPrimitiveMethods<unsigned char>()},

			{literal("pointer"),	getPrimitiveMethods<void*>()},
			{literal("type"),	getPrimitiveMethods<Type>()},

			{literal("string"),	getPrimitiveMethods<string>()},

            {literal("byte_buffer"), getPrimitiveMethods<aio::byte_buffer>()},
            {literal(""),0}
		};
	}

    

	void SetupXirang(Xirang& xr)
	{
        Namespace root = xr.root();

        Namespace sys = NamespaceBuilder().name("sys").adoptBy(root);

		for (TypeDesc* itr(g_systype_table); !itr->name.empty(); ++itr)
		{
            Type t = TypeBuilder(itr->methods)
                .name(itr->name)
                .endBuild()
                .adoptBy(sys);

            TypeAliasBuilder()
                .name(itr->name)
                .typeName(aio::literal(".sys.") << itr->name)
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
