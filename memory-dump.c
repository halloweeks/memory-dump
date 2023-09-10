#include <stdio.h>
#include <time.h>
#include <stdint.h>

#if defined(__ANDROID__)
#include "os_android.h"
#elif defined(__linux__)
#include "os_linux.h"
#else
#error Unknown OS!
#endif

static void memory_dump(uint32_t pid) {
	if (!os_process_begin(pid)) {
		printf("Failed to open process\n");
		return;
	}
	
	printf("Dumping PID %u ...\n", pid);

    char output_path[255];
    snprintf(output_path, sizeof(output_path), "dump-%u.bin", pid);

    FILE *fp = fopen(output_path, "wb");
    if (fp == NULL) {
        printf("Failed to open output file");
        return;
    }

	uint8_t buffer[8192];
    uint64_t pos = 0;

    uint64_t region = 0;
    uint64_t region_size = 0;
    uint64_t size_remaining = 0;
	uint64_t read = 0;

    uint64_t addr = 0;

    clock_t t0 = clock();
    uint64_t total = 0;

    for (;;)
    {

        if (size_remaining == 0)
        {
            uint64_t next_size;
            uint64_t next = os_process_next(&next_size);
            
            if (next == 0)
            {
                break;
            }

            if (region + region_size != next)
            {
                pos = 0;
            }

            addr = region = next;
            size_remaining = region_size = next_size;
        }
		
		read = 8192;
		if (read > size_remaining) {
			read = size_remaining;
		}
        read = os_process_read(region, buffer, (uint32_t)read);

        if (read == 0)
        {
            pos = 0;
            size_remaining = 0;

            continue;
		}
		else {
			// Write out the region
			fwrite(buffer, 1, read, fp);
		}

        total += read;
        region += read;
        size_remaining -= read;
        pos += read;
    }

    clock_t t1 = clock();
    double time = (double)(t1 - t0) / CLOCKS_PER_SEC;
    const double MB = 1024.0 * 1024.0;
    printf("Processed %.2f MB, speed = %.2f MB/s\n", total / MB, total / MB / time);

    fclose(fp);
    os_process_end();
}


int main(int argc, const char *argv[]) {
	if (argc < 3) {
		printf("Usage: mem-dump -p pid | -n process-name\n");
		return 0;
	}
	
	os_startup();
	
	uint32_t pid = 0;
	
	if (strcmp(argv[1], "-p") == 0) {
		pid = atoi(argv[2]);
		memory_dump(pid);
	} else if (strcmp(argv[1], "-n") == 0) {
		if (os_enum_start()) {
			while ((pid = os_enum_next(argv[2])) != 0) {
				memory_dump(pid);
			}
			os_enum_end();
        }
	} else {
		printf("Invalid option: %s\n", argv[1]);
		return 1;
	}
	
	
	return 0;
}