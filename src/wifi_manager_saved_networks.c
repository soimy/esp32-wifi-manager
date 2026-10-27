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

bool wifi_manager_saved_networks_is_legacy(size_t blob_size, size_t max_entries,
					   size_t entry_size)
{
	return entry_size > 0 && max_entries > 0 &&
	       blob_size > max_entries * entry_size;
}

size_t wifi_manager_saved_networks_merge(const void *current,
					 const void *existing, size_t existing_count,
					 size_t entry_size, size_t ssid_size,
					 size_t max_entries, void *out_entries,
					 size_t out_size)
{
	const unsigned char *current_entry = (const unsigned char *)current;
	const unsigned char *entries = (const unsigned char *)existing;
	unsigned char *out = (unsigned char *)out_entries;
	size_t capacity;
	size_t count;
	size_t i;

	if (current_entry == NULL || current_entry[0] == 0 || out == NULL ||
	    entry_size == 0 || ssid_size == 0 || ssid_size > entry_size)
	{
		return 0;
	}

	capacity = out_size / entry_size;
	if (capacity > max_entries)
	{
		capacity = max_entries;
	}
	if (capacity == 0)
	{
		return 0;
	}

	memcpy(out, current_entry, entry_size);
	count = 1;

	if (entries == NULL)
	{
		return count;
	}

	for (i = 0; i < existing_count && count < capacity; i++)
	{
		const unsigned char *candidate = entries + i * entry_size;
		size_t j;
		bool duplicate = false;

		if (candidate[0] == 0)
		{
			continue;
		}
		for (j = 0; j < count; j++)
		{
			if (strncmp((const char *)candidate,
				    (const char *)(out + j * entry_size), ssid_size) == 0)
			{
				duplicate = true;
				break;
			}
		}
		if (!duplicate)
		{
			memcpy(out + count * entry_size, candidate, entry_size);
			count++;
		}
	}

	return count;
}

bool wifi_manager_saved_networks_should_persist(bool dont_save, bool migrate,
						bool reorder)
{
	return !dont_save || migrate || reorder;
}
