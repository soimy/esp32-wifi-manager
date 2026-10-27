#ifndef WIFI_MANAGER_SAVED_NETWORKS_H
#define WIFI_MANAGER_SAVED_NETWORKS_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Loads every entry with a non-empty SSID from a saved_networks blob.
 *
 * Blobs written by older firmware hold CONFIG_WIFI_MAX_AP_CONFIGS slots in
 * oldest -> newest order, but that order does not tell which network the
 * device is currently using: reconnecting to an existing SSID updates its slot
 * in place. All entries are therefore kept and compacted into out_entries.
 * The caller persists a single entry only after one of them has connected.
 *
 * out_entries may alias blob for in-place compaction.
 *
 * @param blob        raw NVS blob
 * @param blob_size   blob size in bytes
 * @param entry_size  size of one ap_config_t
 * @param out_entries destination for the compacted entries
 * @param out_size    size of out_entries
 * @param max_entries upper bound on entries to copy
 * @return number of entries copied
 */
size_t wifi_manager_saved_networks_load(const void *blob, size_t blob_size,
					size_t entry_size, void *out_entries,
					size_t out_size, size_t max_entries);

/**
 * @brief Whether a saved_networks blob still uses the legacy multi-slot layout.
 *
 * Old firmware always wrote CONFIG_WIFI_MAX_AP_CONFIGS slots, even when only
 * one network was ever configured, so the blob length is the reliable marker;
 * counting non-empty entries misses the one-network case.
 *
 * @param blob_size  stored blob size in bytes
 * @param entry_size size of one ap_config_t
 * @return true when the blob is larger than one entry
 */
bool wifi_manager_saved_networks_is_legacy(size_t blob_size, size_t entry_size);

/**
 * @brief Decides whether a successful connection must write the saved network.
 *
 * Normal connections always persist. DONT_SAVE connections (boot restore or an
 * explicit USER_NO_SAVE request) only persist when a legacy-format blob must be
 * collapsed after the restore actually connected.
 *
 * @param dont_save true when the connection was started with DONT_SAVE set
 * @param migrate   true when a legacy-format blob must be collapsed
 * @return true when the connected network should be written to NVS
 */
bool wifi_manager_saved_networks_should_persist(bool dont_save, bool migrate);

#endif /* WIFI_MANAGER_SAVED_NETWORKS_H */
