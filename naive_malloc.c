#include "malloc.h"
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/**
 * naive_malloc - Allocates memory.
 * @size: Number of bytes.
 *
 * Return: Pointer to memory, or NULL.
 */
void *naive_malloc(size_t size)
{
	if (size == 0)
		return (NULL);
	size_t max_alignment = alignof(max_align_t);
	size_t h_alignment	 = alignof(size_t);
	long   page_size	 = sysconf(_SC_PAGESIZE);
	size_t page_size_u;

	if (page_size <= 0)
		return (NULL);
	page_size_u			  = (size_t)page_size;
	size_t	  header_size = sizeof(size_t);
	size_t	  required_size, h_padding, p_padding, required_pages;
	size_t	 *header_p	= sbrk(0);
	uintptr_t candidate = 0;
	uintptr_t header	= 0;

	if (size > SIZE_MAX - header_size)
		return (NULL);
	header = (uintptr_t)header_p;
	h_padding =
		header % h_alignment == 0 ? 0 : h_alignment - (header % h_alignment);
	header		  = (uintptr_t)((char *)header_p + h_padding);
	candidate	  = (uintptr_t)(header + header_size);
	p_padding	  = candidate % max_alignment == 0
						? 0
						: (max_alignment - (candidate % max_alignment));
	required_size = header_size + size + h_padding + p_padding;

	if (required_size < page_size_u)
		required_pages = 1;
	required_pages			 = (required_size % page_size_u) == 0
								   ? required_size / page_size_u
								   : (required_size / page_size_u) + 1;
	header_p				 = sbrk(required_pages * page_size_u);
	size_t *aligned_header_p = (size_t *)header;
	*aligned_header_p		 = size;
	void *payload = (char *)(aligned_header_p) + header_size + p_padding;

	return (payload);
}
