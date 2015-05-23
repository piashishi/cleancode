#include "libcache.h"
#include "libcache_def.h"


void* libcache_create(
		libcache_scale_t max_entry_number,
		size_t entry_size,
		size_t key_size,
		LIBCACHE_ALLOCATE_MEMORY* allocate_memory,
		LIBCACHE_FREE_MEMORY* free_memory,
		LIBCACHE_FREE_ENTRY* free_entry,
		LIBCACHE_CMP_KEY* cmp_key,
		LIBCACHE_KEY_TO_NUMBER* key_to_number)
{
	return NULL;
}


void* libcache_lookup(void* libcache, const void* key, void* dst_entry)
{
	return NULL;
}


void* libcache_add(void * libcache, const void* key, const void* src_entry)
{
	return NULL;
}

libcache_ret_t  libcache_delete_by_key(void * libcache, const void* key)
{
	return LIBCACHE_NOT_FOUND;
}

libcache_ret_t  libcache_delete_entry(void * libcache, void* entry)
{
	return LIBCACHE_NOT_FOUND;
}

libcache_ret_t libcache_unlock_entry(void * libcache, void* entry)
{
	return LIBCACHE_NOT_FOUND;
}

libcache_scale_t libcache_get_max_entry_number(const void * libcache)
{
	return 10000;
}

libcache_scale_t libcache_get_entry_number(const void * libcache)
{
	return 1;
}

libcache_ret_t libcache_clean(void * libcache)
{
	return LIBCACHE_FAILURE;
}

libcache_ret_t libcache_destroy(void * libcache)
{
	return LIBCACHE_FAILURE;
}
