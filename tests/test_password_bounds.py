"""Run the production storage/export functions with in-memory NVS/JSON stubs.

Usage: python3 -m unittest discover -s tests
"""

import pathlib
import re
import subprocess
import tempfile
import unittest


def function_source(source, name):
    match = re.search(r"(?m)^[\w *]+\b" + name + r"\([^)]*\)\s*\{", source)
    if match is None:
        raise AssertionError(f"Missing function: {name}")
    end = match.end()
    depth = 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[match.start():end]


STUBS = r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_SSID_SIZE 32
#define MAX_PASSWORD_SIZE 64
#define ESP_OK 0
#define ESP_ERR_INVALID_ARG 1
#define ESP_ERR_NVS_NOT_FOUND 2
#define ESP_LOGE(...) ((void)0)
#define ESP_LOGI(...) ((void)0)
typedef int esp_err_t;
typedef struct { char ssid[32]; char password[64]; } ap_config_t;
typedef struct { ap_config_t entries[10]; size_t count; } wifi_manager_saved_networks_t;
static wifi_manager_saved_networks_t stored;
static esp_err_t wifi_manager_load_saved_networks(wifi_manager_saved_networks_t *out) {
    *out = stored; return ESP_OK;
}
static esp_err_t wifi_manager_write_saved_networks(const ap_config_t *entries, size_t count) {
    assert(count == stored.count);
    memcpy(stored.entries, entries, count * sizeof(*entries)); return ESP_OK;
}
typedef int cJSON;
static cJSON node;
static int json_fields;
static cJSON *cJSON_CreateArray(void) { return &node; }
static cJSON *cJSON_CreateObject(void) { return &node; }
static void cJSON_AddStringToObject(cJSON *obj, const char *key, const char *value) {
    (void)obj;
    assert(strlen(value) == (strcmp(key, "ssid") == 0 ? 32u : 64u));
    json_fields++;
}
static void cJSON_AddItemToArray(cJSON *array, cJSON *item) { (void)array; (void)item; }
static char *cJSON_PrintUnformatted(cJSON *obj) { (void)obj; return "[]"; }
static void cJSON_Delete(cJSON *obj) { (void)obj; }
'''

DRIVER = r'''
int main(int argc, char **argv) {
    assert(argc == 2);
    int length = atoi(argv[1]);
    strcpy(stored.entries[0].ssid, "Home");
    strcpy(stored.entries[1].ssid, "Neighbor");
    stored.count = 2;
    if (length == 0) {
        stored.count = 1;
        memset(stored.entries[0].ssid, 'S', 32);
        memset(stored.entries[0].password, 'a', 64);
        assert(wifi_manager_get_saved_aps_json() != NULL);
        assert(json_fields == 2);
    } else {
        char password[66];
        memset(password, 'a', length);
        password[length] = 0;
        int result = wifi_manager_update_ap_password("Home", password);
        if (length > 64) {
            assert(result == ESP_ERR_INVALID_ARG);
            assert(stored.entries[0].password[0] == 0);
        } else {
            assert(result == ESP_OK);
            assert(memcmp(stored.entries[0].password, password, length) == 0);
            if (length < 64) assert(stored.entries[0].password[length] == 0);
        }
        assert(strcmp(stored.entries[1].ssid, "Neighbor") == 0);
    }
    return 0;
}
'''


class PasswordBoundsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="wifi-password-test-")
        cls.addClassCleanup(cls.temp.cleanup)
        source = (pathlib.Path(__file__).resolve().parents[1] / "src/wifi_manager.c").read_text()
        functions = "\n".join(function_source(source, name) for name in (
            "wifi_manager_update_ap_password", "wifi_manager_get_saved_aps_json"))
        fixture = pathlib.Path(cls.temp.name) / "bounds.c"
        cls.binary = fixture.with_suffix("")
        fixture.write_text(STUBS + functions + DRIVER)
        subprocess.run(["cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
                        "-fsanitize=address,undefined", str(fixture), "-o", str(cls.binary)], check=True)

    def check_case(self, length):
        result = subprocess.run([str(self.binary), str(length)], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_63_byte_password_preserves_neighbor(self):
        self.check_case(63)

    def test_64_byte_psk_preserves_all_bytes_and_neighbor(self):
        self.check_case(64)

    def test_65_byte_password_is_rejected_without_writing(self):
        self.check_case(65)

    def test_json_bounds_full_ssid_and_password(self):
        self.check_case(0)


if __name__ == "__main__":
    unittest.main()
