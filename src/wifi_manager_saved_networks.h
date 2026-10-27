#ifndef WIFI_MANAGER_SAVED_NETWORKS_H
#define WIFI_MANAGER_SAVED_NETWORKS_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Picks the most recent entry with a non-empty SSID out of a saved_networks blob.
 *
 * Blobs written by older firmware hold CONFIG_WIFI_MAX_AP_CONFIGS entries and
 * are ordered oldest -> newest, so the last non-empty entry is the network the
 * device last joined. The caller decides what to do with the selected entry;
 * the wifi manager rewrites it as a single-entry blob.
 *
 * @param blob       raw NVS blob
 * @param blob_size  blob size in bytes
 * @param entry_size size of one ap_config_t
 * @param out_entry  destination for one entry (entry_size bytes are written)
 * @param out_size   size of out_entry
 * @return true when an entry with a non-empty SSID was copied
 */
bool wifi_manager_saved_networks_pick(const void *blob, size_t blob_size,
				      size_t entry_size, void *out_entry,
				      size_t out_size);

#endif /* WIFI_MANAGER_SAVED_NETWORKS_H */
