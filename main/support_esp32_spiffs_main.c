/*
 * HARDWARE SETUP the MJD components:
 *  *NONE
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/dirent.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_spiffs.h"
#include "esp_err.h"
#include "esp_log.h"

#define RTOS_DELAY_10MILLISEC (10 / portTICK_PERIOD_MS)
#define RTOS_DELAY_100MILLISEC (100 / portTICK_PERIOD_MS)
#define RTOS_DELAY_1SEC (1 * 1000 / portTICK_PERIOD_MS)

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * LOGGING SPIFFS
 *  @doc The partition is not overwritten when writing a new application to flash (it is in another sector of the Flash EEPROM)
 */
static FILE* _log_file;
static const char SPIFFS_PARTITION[] = "myspiffs";
static const char SPIFFS_BASE_PATH[] = "/spiffs";
static const char LOG_PATH[] = "/spiffs/log.txt";

esp_err_t mjdlib_log_format_spiffs() {
    ESP_LOGI(TAG, "SPIFFS Formatting...");
    if (esp_spiffs_format(SPIFFS_PARTITION) != ESP_OK) {
        ESP_LOGE(TAG, "Format failed");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Format OK");

    return ESP_OK;
}

esp_err_t mjdlib_log_df() { // ~bash df (disk free)
    esp_err_t ret;
    size_t total_bytes = 0, used_bytes = 0;

    ret = esp_spiffs_info(NULL, &total_bytes, &used_bytes);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "df. Failed to get SPIFFS partition information");
        return ret;
    }
    ESP_LOGI(TAG, "df. Partition size of %s: total: %u, used: %u (%u%%)", SPIFFS_PARTITION, total_bytes, used_bytes, 100*used_bytes/total_bytes);

    return ESP_OK;
}

esp_err_t mjdlib_log_ls() {

    char dashes[255 + 1] = "";
    memset(dashes, '-', 255);

    printf("%s %s\n", "ls", SPIFFS_BASE_PATH);
    printf("%10.10s  %20.20s  %50.50s\n", "Type", "Size", "Name");
    printf("%10.10s  %10.20s  %50.50s\n", dashes, dashes, dashes);

    struct stat st;
    DIR *dir;
    dir = opendir(SPIFFS_BASE_PATH);
    if (!dir) {
        ESP_LOGE(TAG, "ls. Error opening directory");
        return ESP_FAIL;
    }
    char uri[512];
    struct dirent *direntry;
    while ((direntry = readdir(dir)) != NULL) {
        switch (direntry->d_type) {
        case DT_DIR:  // A directory.
            printf("%10s  %20s  %50s\n", "DIR", "", direntry->d_name);
            break;
        case DT_REG:  // A regular file.
            printf("%10s  ", "FILE");
            sprintf(uri, "%s/%s", SPIFFS_BASE_PATH, direntry->d_name);
            if (stat(uri, &st) == 0) {
                printf("%20ld  ", st.st_size);
            } else {
                printf("%20s  ", "?");
            }
            printf("%50.50s\n", direntry->d_name);
            break;
        case DT_UNKNOWN:  // The type is unknown.
            printf("%10s\n", "UNKNOWN");
            break;
        default:
            printf("%10s\n", "?????");
        }
    }
    closedir(dir);

    printf("\n");

    return ESP_OK;
}


/*
 * MAIN
 */
esp_err_t app_spiffs_logging() {
    esp_err_t ret;

    // Register the SPIFFS filesystem.
    //      @important It only formats the partition when it is not yet formatted (or already formatted but with a different partiton layout such as with the tool mkspiffs)
    esp_vfs_spiffs_conf_t conf = { .base_path = SPIFFS_BASE_PATH, .partition_label = NULL, .max_files = 5, .format_if_mount_failed = true };
    ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_vfs_spiffs_register() failed");
        return ret;
    }

    // df
    mjdlib_log_df();

    // ls
    mjdlib_log_ls();

    //mjdlib_log_format_spiffs();

    // Open log file
    _log_file = fopen(LOG_PATH, "at"); // a=append t=textmode
    if (_log_file == NULL) {
        ESP_LOGE(TAG, "Cannot open logfile");
        return ESP_FAIL;
    }

    // loop append
    static const uint32_t WRITE_CACHE_CYCLE = 5;
    static uint32_t counter_write_cache = 0;
    char dashes[512 + 1] = ""; // >=1024: sometimes stack overflow...
    memset(dashes, '-', 512);
    uint32_t counter_append = 1;
    int iresult;

    while (counter_append <= 1500) {
        // Write to SPIFFS
        if (_log_file == NULL) {
            ESP_LOGE(TAG, "ABORT. file handle _log_file is NULL\n");
            return -1;
        }

        iresult = fprintf(_log_file, "%u %s", counter_append, dashes);
        if (iresult < 0) {
            ESP_LOGE(TAG, "ABORT. failed fprintf() ret %i", iresult);
            break; // EXIT
        }

        // Smart commit after x writes
        counter_write_cache++;
        if (counter_write_cache % WRITE_CACHE_CYCLE == 0) {
            ESP_LOGI(TAG, "fsync'ing (WRITE_CACHE_CYCLE=%u)", WRITE_CACHE_CYCLE);
            fsync(fileno(_log_file));
        }

        // df
        mjdlib_log_df();
        counter_append++;

        vTaskDelay(RTOS_DELAY_10MILLISEC);
    }

    ///
    ///

    // Close log file
    fclose(_log_file);
    _log_file = NULL;

    // df
    mjdlib_log_df();

    // ls
    mjdlib_log_ls();

    // SPIFFS unregister
    ret = esp_vfs_spiffs_unregister(NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. esp_vfs_spiffs_unregister() failed");
        return ret;
    }

    return ESP_OK;
}

void app_main() {
    /**********
     * Logging to SPIFFS
     */
    app_spiffs_logging();
}
