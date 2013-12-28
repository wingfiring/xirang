#include <xirang/type/xirang.h>
#include <xirang/type/typebinder.h>
#include <xirang/type/nativetype.h>
#include <xirang/type/array.h>
#include <xirang/type/itypebinder.h>
#include <xirang/type/nativetypeversion.h>

#include <assert.h>
namespace xirang{ namespace type{
	namespace {
	    template<typename T>
        TypeMethods* getPrimitiveMethods(const string& str)
        {
            static PrimitiveMethods<T> methods(str);
            return &methods;
        }

		struct TypeDesc
		{
			const_range_string name;
			TypeMethods* methods;
		};

		TypeDesc g_systype_table[] =
		{
			{literal("bool"), 	getPrimitiveMethods<bool>(".sys.type.bool")},

			{literal("float"),	getPrimitiveMethods<float>(".sys.type.float")},
			{literal("double"),	getPrimitiveMethods<double>(".sys.type.double")},

			{literal("int8"), 	getPrimitiveMethods<int8_t>(".sys.type.int8")},
			{literal("uint8"), 	getPrimitiveMethods<uint8_t>(".sys.type.uint8")},
			{literal("int16"),	getPrimitiveMethods<int16_t>(".sys.type.uint16")},
			{literal("uint16"),	getPrimitiveMethods<uint16_t>(".sys.type.uint16")},
			{literal("int32"),	getPrimitiveMethods<int32_t>(".sys.type.int32")},
			{literal("uint32"),	getPrimitiveMethods<uint32_t>(".sys.type.uint32")},
			{literal("int64"),	getPrimitiveMethods<int64_t>(".sys.type.int64")},
			{literal("uint64"),	getPrimitiveMethods<uint64_t>(".sys.type.uint64")},

			{literal("byte"),	getPrimitiveMethods<byte>(".sys.type.byte")},

			{literal("pointer"),	getPrimitiveMethods<void*>(".sys.type.pointer")},

			{literal("string"),	getPrimitiveMethods<string>(".sys.type.string")},

            {literal("byte_buffer"), getPrimitiveMethods<byte_buffer>(".sys.type.byte_buffer")},
            {literal(""),0}
		};

		struct AliasPair{
			const_range_string name;
			const_range_string type;
		};
		AliasPair g_alias_table[] =
		{
            {literal("short"), 		literal(".sys.type.int16")},
            {literal("ushort"), 	literal(".sys.type.uint16")},
            {literal("int"), 		literal(".sys.type.int32")},
            {literal("uint"), 		literal(".sys.type.uint32")},
            {literal("long"), 		literal(".sys.type.int64")},
            {literal("ulong"), 		literal(".sys.type.uint64")},
            {literal("llong"), 		literal(".sys.type.int64")},
            {literal("ullong"), 	literal(".sys.type.uint64")},
		};
	}



	void SetupXirang(Xirang& xr)
	{
        Namespace root = xr.root();

        Namespace sys = NamespaceBuilder().name(literal("sys")).adoptBy(root);
        Namespace typeNs = NamespaceBuilder().name(literal("type")).adoptBy(sys);

		for (TypeDesc* itr(g_systype_table); !itr->name.empty(); ++itr)
		{
            Type t = TypeBuilder(itr->methods)
                .name(itr->name)
                .endBuild()
                .adoptBy(typeNs);

            TypeAliasBuilder()
                .name(itr->name)
                .typeName(t.methods().internalID())
                .setType(t)
                .adoptBy(root);
		}

		//static reference type
		{
            Type t = TypeBuilder()
                .name(literal("ref"))
                .setArg(literal("ref_type"), literal(""), Type())
                .addMember(literal("to"), literal("pointer"), typeNs.findType(literal("pointer")))
                .endBuild()
                .adoptBy(typeNs);

            TypeAliasBuilder()
                .name(literal("ref"))
                .typeName(literal(".sys.type.ref"))
                .setType(t)
                .adoptBy(root);
		}

		//dynamic reference type
		{
            Type t = TypeBuilder()
                .name(literal("dynamic_ref"))
                .addMember(literal("to"), literal("pointer"), typeNs.findType(literal("pointer")))
                .addMember(literal("type"), literal("type"), typeNs.findType(literal("type")))
                .endBuild()
                .adoptBy(typeNs);

            TypeAliasBuilder()
                .name(literal("dynamic_ref"))
                .typeName(literal(".sys.type.dynamic_ref"))
                .setType(t)
                .adoptBy(root);
		}
		{
            Type t = TypeBuilder(getPrimitiveMethods<Array>(".sys.type.array"))
                .name(literal("array"))
                .setArg(literal("value_type"), literal(""), Type())
                .endBuild()
                .adoptBy(typeNs);

            TypeAliasBuilder()
                .name(literal("array"))
                .typeName(literal(".sys.type.array"))
                .setType(t)
                .adoptBy(root);
		}
		{
			for (auto& i : g_alias_table){
				Type t = root.locateType(i.type, '.');
				AIO_PRE_CONDITION(t.valid());
				TypeAliasBuilder()
					.name(i.name)
					.typeName(i.type)
					.setType(t)
					.adoptBy(root);
			}
		}
	}
}}

