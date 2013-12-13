#ifndef XIRANG_VERSION_TYPE_H
#define XIRANG_VERSION_TYPE_H
#include <xirang/sha1.h>
namespace xirang{
	enum class hash_algorithm{
		ha_sha1 = 1,
	};
	struct version_type{
		uint16_t protocol_version = 1;
		uint16_t algorithm = uint16_t(hash_algorithm::ha_sha1);
		sha1_digest id;
		uint32_t conflict_id = 0;
	};

	inline bool operator==(const version_type& lhs, const version_type& rhs){
		return lhs.id == rhs.id
			&& lhs.protocol_version == rhs.protocol_version
			&& lhs.algorithm == rhs.algorithm
			&& lhs.conflict_id == rhs.conflict_id;
	}
	inline bool operator<(const version_type& lhs, const version_type& rhs){
		return lhs.id < rhs.id
			|| (lhs.id == rhs.id &&  lhs.protocol_version < rhs.protocol_version)
			|| (lhs.protocol_version == rhs.protocol_version && lhs.algorithm < rhs.algorithm)
			|| (lhs.algorithm == rhs.algorithm && lhs.conflict_id < rhs.conflict_id);
	}

	struct hash_version_type{
		static bool operator()(const version_type& ver){
			return hash_sha1(ver.id);
		}
	};

	struct version_type_compare_ : totally_ordered<version_type>{ };

	template<typename Ar, typename = 
		typename std::enable_if<io::s11n::is_deserializer<Ar>::value>::type>
	Ar load(Ar ar, version_type& ver)
	{
		return ar & ver.protocol_version
			& ver.algorithm
			& ver.id
			& ver.conflict_id;
	}

	template<typename Ar, typename =
		typename std::enable_if< io::s11n::is_serializer<Ar>::value>::type>
	Ar save(Ar ar, const sha1_digest& dig)
	{
		ar & ver.protocol_version
			& ver.algorithm
			& ver.id
			& ver.conflict_id;
		return ar;
	}

}

#endif //end XIRANG_VERSION_TYPE_H


