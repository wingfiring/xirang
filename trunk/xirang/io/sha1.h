#ifndef XIRANG_IO_SHA1_DIGEST_H
#define XIRANG_IO_SHA1_DIGEST_H
#include <xirang/io/s11n.h>
#include <xirang/sha1.h>
namespace xirang{
	template<typename Ar, typename =
		typename std::enable_if<io::s11n::is_deserializer<Ar>::value>::type>
	Ar& load (Ar& ar, sha1_digest& dig)
	{
		for (auto& i : dig.v)
			ar & i;
		return ar;
	}
	template<typename Ar, typename =
		typename std::enable_if< io::s11n::is_serializer<Ar>::value>::type>
	Ar& save(Ar& ar, const sha1_digest& dig)
	{
		for (auto i : dig.v)
			ar & i;
		return ar;
	}


}
#endif //XIRANG_IO_SHA1_DIGEST_H
