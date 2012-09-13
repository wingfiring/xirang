#ifndef AIO_COMMON_ITERATOR_CATEGORY_H
#define AIO_COMMON_ITERATOR_CATEGORY_H

namespace aio
{
	struct incrementable_traversal_tag{};

	struct single_pass_traversal_tag : incrementable_traversal_tag{};

	struct forward_traversal_tag : single_pass_traversal_tag{};

	struct bidirectional_traversal_tag : forward_traversal_tag{};

	struct random_access_iterator_tag : bidirectional_traversal_tag{};

	struct no_io_iterator_tag{};
	struct input_iterator_tag : single_pass_traversal_tag, no_io_iterator_tag{};
	struct output_iterator_tag : single_pass_traversal_tag, no_io_iterator_tag{};

}
#endif //end AIO_COMMON_ITERATOR_CATEGORY_H
