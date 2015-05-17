#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <jansson.h>
#include "isbn.h"
#ifdef _META
	#include "isbn_meta.h"
#endif /* _META */
#ifdef _BARCODE
	#include <png.h>
	#include "isbn_barcode.h"
#endif /* _BARCODE */

static int isbn_barcode_schemes[10] = {
	0x00, 0x0B, 0x0D, 0x0E, 0x13, 0x19, 0x1C, 0x15, 0x16, 0x1A
};

static int isbn_barcode_pattern_left[2][10] = {
	{ 0x0D, 0x19, 0x13, 0x3D, 0x23, 0x31, 0x2F, 0x3B, 0x37, 0x0B },
	{ 0x27, 0x33, 0x1B, 0x21, 0x1D, 0x39, 0x05, 0x11, 0x09, 0x17 }
};

static int isbn_barcode_pattern_right[10] = {
	0x72, 0x66, 0x6C, 0x42, 0x5C, 0x4E, 0x50, 0x44, 0x48, 0x74
};

static int isbn_barcode_font[11][7] = {
	{ 0x1C, 0x22, 0x26, 0x2A, 0x32, 0x22, 0x1C },
	{ 0x08, 0x18, 0x08, 0x08, 0x08, 0x08, 0x1C },
	{ 0x1C, 0x22, 0x02, 0x04, 0x08, 0x10, 0x3C },
	{ 0x3E, 0x04, 0x08, 0x04, 0x02, 0x22, 0x1C },
	{ 0x04, 0x0C, 0x14, 0x24, 0x3E, 0x04, 0x04 },
	{ 0x3E, 0x20, 0x3C, 0x02, 0x02, 0x22, 0x1C },
	{ 0x0C, 0x10, 0x20, 0x3C, 0x22, 0x22, 0x1C },
	{ 0x3E, 0x02, 0x04, 0x08, 0x10, 0x10, 0x10 },
	{ 0x1C, 0x22, 0x22, 0x1C, 0x22, 0x22, 0x1C },
	{ 0x1C, 0x22, 0x22, 0x1E, 0x02, 0x04, 0x18 },
	{ 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10 }
};

void isbn_barcode_digit(char **digit, int code, int length) {
	int i = 0, l = 0, bit = 0;

	*digit[0] = '\0';

	for (i = length - 1, l = 0; i >= l; i--) {
		bit = (code >> i) & 0x01;

		strcat(*digit, bit == 1 ? "1" : "0");
	}
}

void isbn_barcode_digits(char **barcode, char **isbn) {
	int i = 0, l = 0, scheme = 0, guard = 0x15;
	char *digits = NULL, *digit = NULL;

	isbn_to_13(&*isbn);

	*barcode = malloc(sizeof(char) * 95 + 1);
	digits = malloc(sizeof(char) * 7 + 1);
	digit = malloc(sizeof(char) + 1);

	scheme = isbn_barcode_schemes[(int)*isbn[0] - 48];

	isbn_barcode_digit(&digits, guard, 3);
	strcpy(*barcode, digits);

	for (i = 1, l = 7; i < l; i++) {
		strncpy(digit, *isbn + i, 1);
		isbn_barcode_digit(&digits, isbn_barcode_pattern_left[(scheme >> (6 - i)) & 0x01][(int)digit[0] - 48], 7);
		strcat(*barcode, digits);
	}

	isbn_barcode_digit(&digits, guard ^ 0x1F, 5);
	strcat(*barcode, digits);

	for (i = 7, l = 13; i < l; i++) {
		strncpy(digit, *isbn + i, 1);
		isbn_barcode_digit(&digits, isbn_barcode_pattern_right[(int)digit[0] - 48], 7);
		strcat(*barcode, digits);
	}

	isbn_barcode_digit(&digits, guard, 3);
	strcat(*barcode, digits);

	if (digit) {
		free(digit);
	}

	if (digits) {
		free(digits);
	}
}

void isbn_barcode_character(char **character, char digit) {
	int i = 0, j = 0, k = 0, l = 0, code = 0, byte = 0, bit = 0, counter = 0;

	code = ('0' > digit || digit > '9') ? 10 : (int)digit - 48;

	*character[0] = '\0';
	strcpy(*character, "0000000");
	counter = 7;

	for (i = 0, j = 7; i < j; i++) {
		byte = isbn_barcode_font[code][i];

		for (k = 6, l = 0; k >= l; k--) {
			bit = (byte >> k) & 0x01;
			sprintf(*character + counter++, "%d", bit);
		}
	}
}

int isbn_barcode_write(char *barcode, char *isbn, char *filename) {
	int rc = 0, i = 0, l = 0, x = 0, y = 0, counter = 0, offset = 0;
	int width = 0, height = 0, padding_x = 0, padding_y = 0;
	int pixel = 0, **pixels = NULL;
	size_t length = 0;
	char *character = NULL;
	FILE *file = NULL;
	png_byte color_type, bit_depth;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers;

	file = fopen(filename, "wb");

	if (!file) {
		printf("Could not open %s\n", filename);

		rc = 1;
		goto cleanup;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr) {
		printf("PNG write prep failed\n");

		rc = 1;
		goto cleanup;
	}

	info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr) {
		printf("PNG info prep failed\n");

		rc = 1;
		goto cleanup;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("PNG write init failed\n");

		rc = 1;
		goto cleanup;
	}

	png_init_io(png_ptr, file);

	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("PNG header write failed\n");

		rc = 1;
		goto cleanup;
	}

	padding_x = 10;
	padding_y = 4;
	width = 95 + (padding_x * 2);
	height = 72 + padding_y;
	color_type = 2;
	bit_depth = 8;

	png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);

	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height + 1);

	for (y = 0; y < height; y++) {
		row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr) + 1);
	}

	pixels = malloc(sizeof(int *) * height + 1);

	for (y = 0; y < height; y++) {
		pixels[y] = malloc(sizeof(int) * width + 1);

		for (x = 0; x < width; x++) {
			pixels[y][x] = 0;
		}
	}

	for (y = 0, i = 0; y < height - padding_y; y++) {
		for (x = 10; x < width - 10; x++) {
			pixels[y][x] = (int)barcode[i++ % (width - (padding_x * 2))] - 48;
		}
	}

	length = strlen(isbn);
	character = malloc(sizeof(char) * (7 * 8) + 1);

	isbn_barcode_character(&character, isbn[0]);

	for (y = 67, counter = 0; y < 75; y++) {
		for (x = 0; x < 7; x++) {
			pixels[y][x] = (int)character[counter++] - 48;
		}
	}

	isbn_barcode_character(&character, '>');

	for (y = 67, counter = 0; y < 75; y++) {
		for (x = width - 7; x < width; x++) {
			pixels[y][x] = (int)character[counter++] - 48;
		}
	}

	for (i = 0, l = (int)length - 1; i < l; i++) {
		isbn_barcode_character(&character, isbn[i + 1]);

		for (y = 67, counter = 0; y < 75; y++) {
			for (x = (i * 7) + (14 + offset); x < (i * 7) + (21 + offset); x++) {
				pixels[y][x] = (int)character[counter++] - 48;
			}
		}

		if (i == 5) {
			offset = 5;
		}
	}

	for (y = 0, i = 0; y < height; y++) {
        png_byte *row = row_pointers[y];

		for (x = 0; x < width; x++) {
            png_byte *rgb = &(row[x * 3]);

			pixel = (1 - pixels[y][x]) * 255;

			rgb[0] = pixel;
			rgb[1] = pixel;
			rgb[2] = pixel;
		}
	}

	png_write_image(png_ptr, row_pointers);

	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("PNG write end failed\n");

		rc = 1;
		goto cleanup;
	}

	png_write_end(png_ptr, NULL);

cleanup:
	if (file) {
		(void)fclose(file);
	}

	if (pixels) {
		for (y = 0; y < height; y++) {
			if (pixels[y]) {
				free(pixels[y]);
			}
		}

		free(pixels);
	}

	if (character) {
		free(character);
	}

	return rc;
}

int isbn_barcode(char **isbn, char *filename) {
	int rc = 0;
	size_t length = 0;
	char *barcode = NULL, *input = NULL;

	if (!filename) {
		length = strlen(*isbn);
		input = malloc(sizeof(char) * (length + 4) + 1);

		sprintf(input, "%s%s", *isbn, ".png");
	} else {
		input = filename;
	}

	isbn_remove_hyphens(&*isbn);
	isbn_barcode_digits(&barcode, &*isbn);

	if (isbn_barcode_write(barcode, *isbn, input) != 0) {
		printf("Error generating barcode %s\n", input);

		rc = 1;
		goto cleanup;
	}

	printf("Generated barcode %s\n", input);

cleanup:
	if (barcode) {
		free(barcode);
	}

	return rc;
}