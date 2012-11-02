#ifndef AIO_XIRANG_REFERENCE_H
#define AIO_XIRANG_REFERENCE_H

#include <aio/xirang/object.h>

namespace xirang
{
	class Reference
	{
	public:
		Reference();
		explicit Reference(CommonObject obj);
		bool valid() const;
	private:
		CommonObject m_obj;
	};

	class TypedReference
	{
	public:
		Reference();
		/// \pre obj.type() == t
		explicit Reference(Type t, CommonObject obj);
		bool valid() const;
		Type type();
	private:
		Type m_type;
		CommonObject m_obj;
	};
}
#endif //AIO_XIRANG_REFERENCE_H

