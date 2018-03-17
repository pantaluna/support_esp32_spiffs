https://github.com/pantaluna/support_esp32_spiffs

# 0. Problem description
Only 80-87% of a SPIFFS partition on the internal Flash Chip can be used to store data files. 

I refer specifically to percentages of the free space. It does not seem to matter what the actual size in bytes of the SPIFFS partition really is. I tested it with various SPIFFS partition sizes 64KB..1MB. Each time it fails as soon as 80-87% of the SPIFFS partition is used (the percentage varies across partition sizes).

The actual free/used stats are obtained using the func esp_spiffs_info(). I understand that the partition not only has to store the actual characters of the text files, but also various metadata about the file system and its files. I would assume that all of it is covered by the func esp_spiffs_info().

Expected result: I would like to use all the capacity of the SPIFFS partition because the available space is a rare good on MCU's.
 
Possible Cause? I might do something wrong. Or the func esp_spiffs_info() might return incorrect information. Or something else.

# 1. Environment
- MCU: Adafruit HUZZAH32 ESP32 development board.
- MCU: Wemos LOLIN32 Lite development board.
- ESP-IDF Github master branch of Mar 17, 2018.
- A software project whereby metrics are stored in a SPIFFS partition. The metrics are uploaded regularly to a central server for data processing (and one trigger for uploading is when the free space on the partition gets too low).

# 2. Repro Case

## 2.1 MSYS2

````
make erase_flash partition_table flash monitor
````

## 2.2 Logic
The programme will append lines, of 512 characters each, to a text file on the SPIFFS partition. It logs after each append operation the space used/free of the SPIFFS partition. The text file is fsync'd every 5 writes.

The loop stops when the file I/O function returns an error.

Change partitions_support_esp32_spiffs.csv if you want to try another SPIFFS partition size.

## 2.3 The UART output (for all tested SPIFFS partition sizes)
1MB   0x100000	1048576	1024
512MB 0x080000	 524288	 512
64KB  0x010000	  65536	  64

### 1MB 0x100000	1048576	1024
myspiffs,	data,	spiffs,		,			0x100000,

SERIAL UART:

```
I (28) boot: ESP-IDF v3.1-dev-511-g7e268ada 2nd stage bootloader
I (28) boot: compile time 16:59:19
I (28) boot: Enabling RNG early entropy source...
I (34) boot: SPI Speed      : 40MHz
I (38) boot: SPI Mode       : DIO
I (42) boot: SPI Flash Size : 4MB
I (46) boot: Partition Table:
I (50) boot: ## Label            Usage          Type ST Offset   Length
I (57) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (65) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (72) boot:  2 factory          factory app      00 00 00010000 00100000
I (80) boot:  3 myspiffs         Unknown data     01 82 00110000 00100000
I (87) boot: End of partition table
I (91) esp_image: segment 0: paddr=0x00010020 vaddr=0x3f400020 size=0x04eac ( 20140) map
I (107) esp_image: segment 1: paddr=0x00014ed4 vaddr=0x3ffb0000 size=0x02360 (  9056) load
I (113) esp_image: segment 2: paddr=0x0001723c vaddr=0x40080000 size=0x00400 (  1024) load
0x40080000: _iram_start at C:/myiot/esp/esp-idf/components/freertos/xtensa_vectors.S:1685

I (118) esp_image: segment 3: paddr=0x00017644 vaddr=0x40080400 size=0x086d0 ( 34512) load
I (141) esp_image: segment 4: paddr=0x0001fd1c vaddr=0x400c0000 size=0x00000 (     0) load
I (141) esp_image: segment 5: paddr=0x0001fd24 vaddr=0x00000000 size=0x002ec (   748)
I (148) esp_image: segment 6: paddr=0x00020018 vaddr=0x400d0018 size=0x17198 ( 94616) map
0x400d0018: _flash_cache_start at ??:?

I (195) boot: Loaded app from partition at offset 0x10000
I (195) boot: Disabling RNG early entropy source...
I (195) cpu_start: Pro cpu up.
I (199) cpu_start: Starting app cpu, entry point is 0x40080e68
0x40080e68: call_start_cpu1 at C:/myiot/esp/esp-idf/components/esp32/cpu_start.c:225

I (0) cpu_start: App cpu up.
I (209) heap_init: Initializing. RAM available for dynamic allocation:
I (216) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (223) heap_init: At 3FFB2B58 len 0002D4A8 (181 KiB): DRAM
I (228) heap_init: At 3FFE0440 len 00003BC0 (14 KiB): D/IRAM
I (235) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (244) heap_init: At 40088AD0 len 00017530 (93 KiB): IRAM
I (247) cpu_start: Pro cpu start user code
I (266) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
W (272) SPIFFS: mount failed, -10025. formatting...
I (4282) myapp: df. Partition size of myspiffs: total: 956561, used: 0 (0%)
ls /spiffs
      Type                  Size                                                Name
----------  --------------------  --------------------------------------------------

I (4442) myapp: df. Partition size of myspiffs: total: 956561, used: 1004 (0%)
I (4452) myapp: df. Partition size of myspiffs: total: 956561, used: 1506 (0%)
I (4462) myapp: df. Partition size of myspiffs: total: 956561, used: 2008 (0%)
I (4472) myapp: df. Partition size of myspiffs: total: 956561, used: 2510 (0%)
I (4482) myapp: fsync'ing (WRITE_CACHE_CYCLE=5)
I (4482) myapp: df. Partition size of myspiffs: total: 956561, used: 3012 (0%)
.....
I (13522) myapp: df. Partition size of myspiffs: total: 956561, used: 252506 (26%)
.....
I (155682) myapp: df. Partition size of myspiffs: total: 956561, used: 769315 (80%)
E (159582) myapp: ABORT. failed fprintf() ret -1
ls /spiffs
      Type                  Size                                                Name
----------  --------------------  --------------------------------------------------
      FILE                763136                                             log.txt
```

### 512MB 0x080000	524288	512
myspiffs,	data,	spiffs,		,			0x080000,

SERIAL UART:

```
.....
E (69511) myapp: ABORT. failed fprintf() ret -1
I (69511) myapp: df. Partition size of myspiffs: total: 474641, used: 382022 (80%)
ls /spiffs
      Type                  Size                                                Name
----------  --------------------  --------------------------------------------------
      FILE                378752                                             log.txt
```

### 64KB 0x010000	65536	64
myspiffs,	data,	spiffs,		,			0x010000,

SERIAL UART:

```
.....
E (17372) myapp: ABORT. failed fprintf() ret -1
I (17372) myapp: df. Partition size of myspiffs: total: 52961, used: 46184 (87%)
ls /spiffs
      Type                  Size                                                Name
----------  --------------------  --------------------------------------------------
      FILE                 45440                                             log.txt
```


#.
# APPENDICES
#.

# 1. SOP for initial upload to GITHUB
https://github.com/pantaluna/support_esp32_spiffs

## 1.a: BROWSER: create github public repo support_esp32_spiffs of pantaluna at Github.com

## 1.b: MSYS2 git

```
git config --system --unset credential.helper
git config credential.helper store
git config --list

cd  /c/myiot/esp/support_esp32_spiffs
git init
git add .
git commit -m "First commit"
git remote add origin https://github.com/pantaluna/support_esp32_spiffs.git
git push --set-upstream origin master

git remote show origin
git status

git tag --annotate v1.0 --message "The original bug report"
git push origin --tags

```

# 2. SOP for source updates

```
cd  /c/myiot/esp/support_esp32_spiffs
git add .
git commit -m "Another commit"
git push --set-upstream origin master

git status
```

