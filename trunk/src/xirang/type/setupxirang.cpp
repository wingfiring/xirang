#include <xirang/xirang.h>
#include <xirang/typebinder.h>
#include <xirang/nativetype.h>
#include <xirang/io/string.h>
#include <xirang/array.h>

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

        Namespace sys = NamespaceBuilder().name(literal("sys")).adoptBy(root);

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
                .name(literal("ref"))
                .setArg(literal("ref_type"), literal(""), Type())
                .addMember(literal("to"), literal("pointer"), sys.findType(literal("pointer")))
                .endBuild()
                .adoptBy(sys);

            TypeAliasBuilder()
                .name(literal("ref"))
                .typeName(literal(".sys.ref"))
                .setType(t)
                .adoptBy(root);
		}

		//dynamic reference type
		{	
            Type t = TypeBuilder()
                .name(literal("dynamic_ref"))
                .addMember(literal("to"), literal("pointer"), sys.findType(literal("pointer")))
                .addMember(literal("type"), literal("type"), sys.findType(literal("type")))
                .endBuild()
                .adoptBy(sys);

            TypeAliasBuilder()
                .name(literal("dynamic_ref"))
                .typeName(literal(".sys.dynamic_ref"))
                .setType(t)
                .adoptBy(root);
		}
		{
            Type t = TypeBuilder(getPrimitiveMethods<Array>())
                .name(literal("array"))
                .setArg(literal("value_type"), literal(""), Type())
                .endBuild()
                .adoptBy(sys);

            TypeAliasBuilder()
                .name(literal("array"))
                .typeName(literal(".sys.array"))
                .setType(t)
                .adoptBy(root);
		}
	}
}
