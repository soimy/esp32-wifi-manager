#include "wifi_manager_saved_networks.h"

#include <string.h>

bool wifi_manager_saved_networks_pick(const void *blob, size_t blob_size,
				      size_t entry_size, void *out_entry,
				      size_t out_size)
{
	const unsigned char *entries = (const unsigned char *)blob;
	size_t offset;
	bool found = false;

	if (entries == NULL || out_entry == NULL || entry_size == 0 ||
	    entry_size > out_size)
	{
		return false;
	}

	/* Legacy blobs are ordered oldest -> newest; keep the newest entry so an
	 * upgrade does not drop the network the device is currently using. */
	for (offset = 0; offset + entry_size <= blob_size; offset += entry_size)
	{
		if (entries[offset] != 0)
		{
			memcpy(out_entry, entries + offset, entry_size);
			found = true;
		}
	}

	return found;
}
