#include "malloc.h"
/* sysconf sbrk */
#include <unistd.h>
/* fprintf perror */
#include <stdio.h>

#define ALIGN sizeof(void *)

/**
 * addPageToHeap - adds one page of memory to heap
 *
 * Return: pointer to previous program break; or NULL on failure
 */
void *addPageToHeap(void)
{
	long int page_sz;
	void	*ptr;

	page_sz = sysconf(_SC_PAGESIZE);
	if (page_sz == -1)
	{
		fprintf(stderr, "addPageToHeap: sysconf failure\n");
		return (NULL);
	}

	ptr = sbrk((intptr_t)page_sz);
	if (ptr == (void *)-1)
	{
		perror("addPageToHeap: sbrk");
		return (NULL);
	}

	return (ptr);
}

/**
 * findUnusedBlock - finds first unused block in naive malloc
 *   implemententation
 *
 * @blk_0_addr: pointer to first block / original program break
 * @size: pointer to amount of bytes user requested; modified by reference to
 *   ensure resulting block will be aligned
 * @used_blk_ct: amount of allocated blocks in naive malloc implementation
 * @unused_blk_addr: address in heap of unused region of heap; modifed by
 *   reference upon finding unused block of suitable size
 * @unused_blk_sz: size in bytes of unsued region of heap; modifed by
 *   reference upon finding unused block of suitable size
 * Return: 0 on success, 1 on failure
 */
int findUnusedBlock(void *blk_0_addr, size_t *size, size_t used_blk_ct,
					void **unused_blk_addr, size_t *unused_blk_sz)
{
	void	*blk_addr;
	long int page_sz;
	size_t	 i, blk_sz;

	if (blk_0_addr == NULL || size == NULL || unused_blk_addr == NULL ||
		unused_blk_sz == NULL)
		return (1);

	page_sz = sysconf(_SC_PAGESIZE);
	if (page_sz == -1)
	{
		fprintf(stderr, "findUnusedBlock: sysconf failure\n");
		return (1);
	}

	/* find next unused block */
	for (blk_addr = blk_0_addr, i = 0; i < used_blk_ct; i++)
	{
		blk_sz	 = *((size_t *)blk_addr);
		blk_addr = (unsigned char *)blk_addr + blk_sz;
	}

	*unused_blk_addr = blk_addr;
	*unused_blk_sz =
		used_blk_ct ? *((size_t *)*unused_blk_addr) : (size_t)page_sz;

	/* presumes alignment of starting progam break and previous blocks */
	*size += (ALIGN - (*size % ALIGN));

	/* extend unused region if too small for new block + unused header */
	while (*unused_blk_sz < (sizeof(size_t) * 2) + *size)
	{
		if (addPageToHeap() == NULL)
			return (1);
		*unused_blk_sz += page_sz;
		*((size_t *)*unused_blk_addr) = *unused_blk_sz;
	}

	return (0);
}

/**
 * naive_malloc - naive implementation of memory allocation in heap, new
 *   blocks are allocated by pushing program break forward and no list of free
 *   blocks is maintained for reuse
 *
 * @size: size of memory requested by user, in bytes
 * Return: pointer to first byte in a contiguous region of `size`
 *   bytes in the heap; aligned for any kind of variable
 */
void *naive_malloc(size_t size)
{
	static void	 *blk_0_addr;
	static size_t used_blk_ct;
	void		 *brk_pt, *payload_addr, *new_blk_addr, *unused_blk_addr;
	size_t		  new_blk_sz, unused_blk_sz;

	/* init starting address on first call */
	if (!blk_0_addr)
	{
		blk_0_addr = addPageToHeap();
		if (blk_0_addr == NULL)
			return (NULL);
	}

	brk_pt = sbrk(0);
	if (brk_pt == (void *)-1)
	{
		perror("naive_malloc: sbrk");
		return (NULL);
	}

	if (findUnusedBlock(blk_0_addr, &size, used_blk_ct, &unused_blk_addr,
						&unused_blk_sz) == 1)
		return (NULL);

	/* set new block for use at previous start of unused block */
	new_blk_addr			  = unused_blk_addr;
	new_blk_sz				  = sizeof(size_t) + size;
	*((size_t *)new_blk_addr) = new_blk_sz;
	used_blk_ct++;

	/* set new start of unused block after new block */
	unused_blk_sz -= new_blk_sz;
	unused_blk_addr = (unsigned char *)unused_blk_addr + new_blk_sz;
	*((size_t *)unused_blk_addr) = unused_blk_sz;

	payload_addr = (unsigned char *)new_blk_addr + sizeof(size_t);
	return (payload_addr);
}

