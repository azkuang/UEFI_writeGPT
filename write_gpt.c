#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint32_t time_lo;
	uint16_t time_mid;
	uint16_t time_hi_and_ver;  // higest 4 bits are version
	uint8_t clock_seq_hi_and_res; // highest bits are variant #
	uint8_t clock_seq_lo;
	uint8_t node[6];
} __attribute__ ((packed)) Guid;

typedef struct {
	uint8_t boot_indicator;
	uint8_t starting_chs[3];
	uint8_t os_type;
	uint8_t ending_chs[3];
	uint32_t starting_lba;
	uint32_t size_lba;
} __attribute__ ((packed)) Mbr_Partition;

typedef struct {
	uint8_t boot_code[440];
	uint32_t mbr_signature;
	uint16_t unknown;
	Mbr_Partition partition[4];
	uint16_t boot_signature;
} __attribute__ ((packed)) Mbr;

typedef struct {
	uint8_t signature[8];
	uint32_t revision;
	uint32_t header_size;
	uint32_t header_crc32;
	uint32_t reserve_1;
	uint64_t my_lba;
	uint64_t alternative_lba;
	uint64_t first_usable_lba;
	uint64_t last_usable_lba;
	Guid disk_guid;
	uint64_t partition_table_lba;
	uint32_t number_of_entries;
	uint32_t size_of_entry;
	uint32_t partition_table_crc32;
	uint8_t reserved_2[512-92];
} __attribute__ ((packed)) Gpt_headers;

char *image_name = "test.img";
uint64_t lba_size = 512;
uint64_t esp_size = 1024*1024*33;
uint64_t data_size = 1024*1024*1;
uint64_t image_size = 0;
uint64_t esp_lbas, data_lbas, image_size_lbas;

uint64_t bytes_to_lbas(const uint64_t bytes)
{
	return (bytes / lba_size) + (bytes % lba_size > 0 ? 1 : 0);
}

void write_full_lba_size(FILE *image)
{
	uint8_t zero_sector[512];
	for (uint8_t i = 0; i < (lba_size - sizeof(zero_sector)) / sizeof(zero_sector); i++)
	{
		fwrite(zero_sector, sizeof(zero_sector), 1, image);
	}
}

bool write_mbr(FILE *image)
{
	uint64_t mbr_size_lbas = image_size_lbas;
	if (mbr_size_lbas > 0xFFFFFFFF) mbr_size_lbas = 0x100000000;
	Mbr mbr = {
		.boot_code = { 0 },
		.mbr_signature = 0,
		.unknown = 0,
		.partition[0] = {
			.boot_indicator = 0,
			.starting_chs = { 0x00, 0x02, 0x00 },
			.os_type = 0xEE,
			.ending_chs = { 0xFF, 0xFF, 0xFF },
			.starting_lba = 0x00000001,
			.size_lba = mbr_size_lbas - 1, 

		},
		.boot_signature = 0xAA55,
	};

	if (fwrite(&mbr, 1, sizeof mbr, image) != sizeof(mbr)) {
		return false;
	}

	write_full_lba_size(image);

	return true;
}

Guid new_guid(void)
{
	uint8_t guid_array[16] = { 0 };
	for (uint8_t i = 0; i < sizeof(guid_array); i++) {
		guid_array[i] = rand() % (UINT8_MAX + 1);
	}

	// TODO: Fill out GUID
	Guid results = {
		.time_lo = *(uint32_t *)&guid_array[0],
		.time_mid = *(uint32_t *)&guid_array[4],
		.time_hi_and_ver = *(uint32_t *)&guid_array[6],
		.clock_seq_hi_and_res = guid_array[8],
		.clock_seq_lo = guid_array[9],
		.node = {
			guid_array[10],
			guid_array[11],
			guid_array[12],
			guid_array[13],
			guid_array[14],
			guid_array[15] 
		},

	};

	// TODO: fill in version bits
	.time_hi_and_ver |= 
	.time_hi_and_ver |=
	.time_hi_and_ver |=
	// TODO: fill in variant bits
	return results;
}

bool write_gpt(FILE *image)
{
	Gpt_headers primary_gpt = {
		.signature = { "EFI PART" },
		.revision = 0x00010000,
		.header_size = 92, // Calculate later
		.header_crc32 = 0, // Calculate later
		.reserve_1 = 0,
		.my_lba = 1,
		.alternative_lba = image_size_lbas - 1,
		.first_usable_lba = 1 + 1 + 32,
		.last_usable_lba = image_size_lbas - 1 - 32 - 1,
		.disk_guid = new_guid(), // Make this function later
		.partition_table_lba = 2,
		.number_of_entries = 128,
		.size_of_entry = 128,
		.partition_table_crc32 = 0, // Calculate later
		.reserved_2 = { 0 },
	};

	// TODO: Make function for CRC32 calc
	// TODO: Make function for GUID
	// TODO: Fill out primary table partition entries
	// TODO: Fill out primary table CRC value
	// TODO: Write primary header to file
	// TODO: Write primary table to file
	//
	// TODO: Fill out secondary GPT header (copy of the primary)
	// TODO: Go to position of secondary table
	// TODO: Write secondary table to file
	// TODO: 
	return true;
}

int main(void)
{
	FILE *image = fopen(image_name, "wb+");
	if (!image) {
		fprintf(stderr, "Error: could not open file %s.\n", image_name);
		return EXIT_FAILURE;
	}

	image_size = esp_size + data_size + (1024*1024); // Add extra padding for GPT/MBR
	image_size_lbas = bytes_to_lbas(image_size);

	if (!write_mbr(image)) {
		fprintf(stderr, "Error: could not write to protective MBR for %s.\n", image_name);
		return EXIT_FAILURE;
	}

	if (!write_gpt(image)) {
		fprintf(stderr, "Error: could not write GPT headers and table for %s.\n", image_name);
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}
