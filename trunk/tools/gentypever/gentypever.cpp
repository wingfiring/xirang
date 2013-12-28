#include <xirang/type/xirang.h>
#include <xirang/type/type.h>
#include <xirang/type/typebinder.h>
#include <xirang/type/nativetype.h>
#include <xirang/io/string.h>
#include <xirang/io/buffer.h>
#include <xirang/type/array.h>

#include <iostream>
#include <iomanip>
using namespace xirang;
using namespace xirang::type;

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

		{literal("int8_t"), 	getPrimitiveMethods<int8_t>(".sys.type.int8")},
		{literal("uint8_t"), 	getPrimitiveMethods<uint8_t>(".sys.type.uint8")},
		{literal("int16_t"),	getPrimitiveMethods<int16_t>(".sys.type.int16")},
		{literal("uint16_t"),	getPrimitiveMethods<uint16_t>(".sys.type.uint16")},
		{literal("int32_t"),	getPrimitiveMethods<int32_t>(".sys.type.int32")},
		{literal("uint32_t"),	getPrimitiveMethods<uint32_t>(".sys.type.uint32")},
		{literal("int64_t"),	getPrimitiveMethods<int64_t>(".sys.type.int64")},
		{literal("uint64_t"),	getPrimitiveMethods<uint64_t>(".sys.type.uint64")},

		{literal("long long"),	getPrimitiveMethods<long long>(".sys.type.int64")},
		{literal("unsigned long long"),	getPrimitiveMethods<unsigned long long>(".sys.type.uint64")},

		{literal("byte"),	getPrimitiveMethods<byte>(".sys.type.byte")},

		{literal("void*"),	getPrimitiveMethods<void*>(".sys.type.pointer")},
		//{literal("type"),	getPrimitiveMethods<Type>(".sys.type.type")},

		{literal("string"),	getPrimitiveMethods<string>(".sys.type.string")},

		{literal("byte_buffer"), getPrimitiveMethods<byte_buffer>(".sys.type.byte_buffer")},
		{literal(""),0}
	};
}
void print(Type t){
	std::cout << "XIRANG_DEFINE_TYPE_VERSION_OF(\""
		<< (t.methods().getTypeVersion(t).id.to_string()) << "\", "
		<< std::setw(12) << t.name() << ", \"" << t.methods().internalID() <<"\");\n";
}

int main(){
	sha1 sha;
	std::cout << sha.get_digest() << std::endl;
	Xirang xr("demo", memory::get_global_heap(), memory::get_global_ext_heap());
	{
		Namespace root = xr.root();

		Namespace sys = NamespaceBuilder().name(literal("sys")).adoptBy(root);

		for (TypeDesc* itr(g_systype_table); !itr->name.empty(); ++itr)
		{
			Type t = TypeBuilder(itr->methods)
				.name(itr->name)
				.endBuild()
				.adoptBy(sys);
			print(t);
		}
		{
			Type t = TypeBuilder(getPrimitiveMethods<Array>(".sys.type.array"))
				.name(literal("Array"))
				.setArg(literal("value_type"), literal(""), Type())
				.endBuild()
				.adoptBy(sys);

			print(t);
		}
	}
}
