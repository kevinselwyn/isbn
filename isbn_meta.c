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

#define PUBLISHER_0      1565
#define PUBLISHER_1      777

int isbn_load_data(struct isbn_data_struct *data, char *filename) {
    int rc = 0, i = 0, j = 0, l = 0, count = 0, pos = 0, total = 0;
    size_t filesize = 0;
    char *filedata = NULL;
    FILE *file = NULL;
    
    file = fopen(filename, "rb");
    
    if (!file) {
        printf("Could not open %s\n", filename);
        
        rc = 1;
        goto cleanup;
    }
    
    (void)fseek(file, 0, SEEK_END);
    filesize = (size_t)ftell(file);
    (void)fseek(file, 0, SEEK_SET);
    
    if (filesize == 0) {
        printf("%s is empty\n", filename);
        
        rc = 1;
        goto cleanup;
    }
    
    filedata = malloc(sizeof(char) * filesize + 1);
    
    if (!filedata) {
        printf("Memory error\n");
        
        rc = 1;
        goto cleanup;
    }
    
    if (fread(filedata, 1, filesize, file) != filesize) {
        printf("Could not read %s\n", filename);
        
        rc = 1;
        goto cleanup;
    }
    
    count = 0;
    
    for (i = 0, l = (int)filesize; i < l; i++) {
        if (filedata[i] == '\n') {
            count++;
        }
    }
    
    data->data_count = count;
    data->data = malloc(sizeof(char **) * count + 1);

    for (i = 0, j = count; i < j; i++) {
		data->data[i] = malloc(sizeof(char *) * 2 + 1);
        
        while (filedata[pos] != ',') {
            pos++;
        }
        
		data->data[i][0] = malloc(sizeof(char) * (pos - total) + 1);
        strncpy(data->data[i][0], filedata + total, pos - total);
        
        total += pos++ - total + 1;
        
        while (filedata[pos] != '\n') {
            pos++;
        }

		data->data[i][1] = malloc(sizeof(char) * (pos - total) + 1);
        strncpy(data->data[i][1], filedata + total, pos - total);
        
        total += pos++ - total + 1;

        if (total >= (int)filesize) {
            break;
        }
    }
    
cleanup:
    if (filedata) {
        free(filedata);
    }
    
    if (file) {
        (void)fclose(file);
    }
    
    return rc;
}

void isbn_free_data(struct isbn_data_struct *data) {
	int i = 0, l = 0;

	if (data->data) {
		for (i = 0, l = data->data_count; i < l; i++) {
			if (data->data[i]) {
				if (data->data[i][0]) {
					free(data->data[i][0]);
				}

				if (data->data[i][1]) {
					free(data->data[i][1]);
				}

				free(data->data[i]);
			}
		}

		free(data->data);
		data->data_count = 0;
	}
}

void isbn_identifier(char **identifier, char **identifier_code, char **isbn) {
	int i = 0, l = 0;
	size_t length = 0;

	isbn_remove_hyphens(&*isbn);

	if (strncmp(*isbn, "9791", 4) != 0) {
		isbn_to_10(&*isbn);
	}

	for (i = isbn_identifiers.data_count - 1, l = 0; i >= l; i--) {
		length = strlen(isbn_identifiers.data[i][0]);

		if (strncmp(*isbn, isbn_identifiers.data[i][0], length) == 0) {
			*identifier_code = malloc(sizeof(char) * length + 1);
			strncpy(*identifier_code, isbn_identifiers.data[i][0], length + 1);

			length = strlen(isbn_identifiers.data[i][1]);

			*identifier = malloc(sizeof(char) * length + 1);
			strncpy(*identifier, isbn_identifiers.data[i][1], length + 1);

			break;
		}
	}

	if (!*identifier) {
		*identifier = malloc(sizeof(char) * 7 + 1);
		strncpy(*identifier, "Unknown", 7 + 1);
	}
}

void isbn_publisher(char **publisher, char **publisher_code, char **isbn) {
	int i = 0, l = 0, publisher_id = 0;
	size_t length = 0, identifier_length = 0;
	char *identifier = NULL, *identifier_code = NULL;

	isbn_identifier(&identifier, &identifier_code, &*isbn);

	identifier_length = strlen(identifier_code);

	if (strncmp(identifier_code, "0", 1) == 0) {
		publisher_id = 0;
	} else if (strncmp(identifier_code, "1", 1) == 0) {
		publisher_id = 1;
	} else {
		goto cleanup;
	}

	i = isbn_publishers[publisher_id].data_count - 1;

	for (l = 0; i >= l; i--) {
		length = strlen(isbn_publishers[publisher_id].data[i][0]);

		if (strncmp(*isbn + identifier_length, isbn_publishers[publisher_id].data[i][0], length) == 0) {
			*publisher_code = malloc(sizeof(char) * length + 1);
			strncpy(*publisher_code, isbn_publishers[publisher_id].data[i][0], length + 1);

			length = strlen(isbn_publishers[publisher_id].data[i][1]);

			*publisher = malloc(sizeof(char) * length + 1);
			strncpy(*publisher, isbn_publishers[publisher_id].data[i][1], length + 1);

			break;
		}
	}

	if (!*publisher) {
		*publisher = malloc(sizeof(char) * 7 + 1);
		strncpy(*publisher, "Unknown", 7 + 1);
	}

cleanup:
	if (identifier) {
		free(identifier);
	}

	if (identifier_code) {
		free(identifier_code);
	}
}

void isbn_book(char **book, char **isbn) {
	size_t code_length = 0, length = 0;
	char *identifier = NULL, *identifier_code = NULL;
	char *publisher = NULL, *publisher_code = NULL;

	isbn_identifier(&identifier, &identifier_code, &*isbn);
	isbn_publisher(&publisher, &publisher_code, &*isbn);

	code_length = strlen(identifier_code) + strlen(publisher_code);
	length = strlen(*isbn) - code_length - 1;

	*book = malloc(sizeof(char) * length + 1);
	strncpy(*book, *isbn + code_length, length);

	if (identifier) {
		free(identifier);
	}

	if (identifier_code) {
		free(identifier_code);
	}

	if (publisher) {
		free(publisher);
	}

	if (publisher_code) {
		free(publisher_code);
	}
}