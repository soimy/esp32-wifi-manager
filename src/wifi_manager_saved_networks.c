#include "wifi_manager_saved_networks.h"

#include <string.h>

size_t wifi_manager_saved_networks_load(const void *blob, size_t blob_size,
					size_t entry_size, void *out_entries,
					size_t out_size, size_t max_entries)
{
	const unsigned char *entries = (const unsigned char *)blob;
	unsigned char *out = (unsigned char *)out_entries;
	size_t capacity;
	size_t count = 0;
	size_t offset;

	if (entries == NULL || out == NULL || entry_size == 0)
	{
		return 0;
	}

	capacity = out_size / entry_size;
	if (capacity > max_entries)
	{
		capacity = max_entries;
	}

	for (offset = 0; offset + entry_size <= blob_size && count < capacity;
	     offset += entry_size)
	{
		if (entries[offset] != 0)
		{
			/* memmove: the caller may compact the blob in place */
			memmove(out + count * entry_size, entries + offset, entry_size);
			count++;
		}
	}

	return count;
}

bool wifi_manager_saved_networks_is_legacy(size_t blob_size, size_t entry_size)
{
	return entry_size > 0 && blob_size > entry_size;
}

bool wifi_manager_saved_networks_should_persist(bool dont_save, bool migrate)
{
	return !dont_save || migrate;
}
