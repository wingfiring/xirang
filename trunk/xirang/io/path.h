#ifndef XIRANG_IO_PATH_H
#define XIRANG_IO_PATH_H
#include <xirang/io/s11n.h>
#include <xirang/path.h>
namespace xirang{
	template<typename Ar, typename =
		typename std::enable_if<io::s11n::is_deserializer<Ar>::value>::type>
	Ar& load (Ar& ar, file_path& path)
	{
		string p = load<string>(ar);
		path = file_path(p, pp_none);
		return ar;
	}
	template<typename Ar, typename =
		typename std::enable_if< io::s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const file_path& path)
	{
		return ar & path.str();
	}

	template<typename Ar, typename =
		typename std::enable_if<io::s11n::is_deserializer<Ar>::value>::type>
	Ar& load (Ar& ar, simple_path& path)
	{
		string p = load<string>(ar);
		path = simple_path(p, pp_none);
		return ar;
	}
	template<typename Ar, typename =
		typename std::enable_if< io::s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const simple_path& path)
	{
		return ar & path.str();
	}
}
#endif //end XIRANG_IO_PATH_H

