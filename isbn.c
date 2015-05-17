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

static char *isbn_api = "https://www.googleapis.com/books/v1/volumes?q=isbn:";
static char *isbn_cache = "cache/";

void isbn_dump_volume(struct isbn_volume volume, char *lookup) {
	int is_lookup = lookup == NULL ? 1 : 0;
	char *name = NULL;

	if (volume.title != NULL) {
		if (lookup == NULL || strncmp(lookup, "title", 5) == 0) {
			name = lookup == NULL ? "Title:" : "";
			printf("%s%*s%s\n", name, 8 * is_lookup, "", volume.title);
		}
	}

	if (volume.subtitle != NULL) {
		if (lookup == NULL || strncmp(lookup, "subtitle", 8) == 0) {
			name = lookup == NULL ? "Subtitle:" : "";
			printf("%s%*s%s\n", name, 5 * is_lookup, "", volume.subtitle);
		}
	}

	if (volume.author != NULL) {
		if (lookup == NULL || strncmp(lookup, "author", 6) == 0) {
			name = lookup == NULL ? "Author:" : "";
			printf("%s%*s%s\n", name, 7 * is_lookup, "", volume.author);
		}
	}

	if (volume.publisher != NULL) {
		if (lookup == NULL || strncmp(lookup, "publisher", 9) == 0) {
			name = lookup == NULL ? "Publisher:" : "";
			printf("%s%*s%s\n", name, 4 * is_lookup, "", volume.publisher);
		}
	}

	if (volume.publishedDate != NULL) {
		if (lookup == NULL || strncmp(lookup, "publishedDate", 13) == 0) {
			name = lookup == NULL ? "Publish Date:" : "";
			printf("%s%*s%s\n", name, is_lookup, "", volume.publishedDate);
		}
	}

	if (volume.pageCount != 0) {
		if (lookup == NULL || strncmp(lookup, "pageCount", 9) == 0) {
			name = lookup == NULL ? "Page Count:" : "";
			printf("%s%*s%d\n", name, 3 * is_lookup, "", volume.pageCount);
		}
	}

	if (volume.description != NULL) {
		if (lookup == NULL || strncmp(lookup, "description", 11) == 0) {
			name = lookup == NULL ? "Description:\n" : "";
			printf("%s%s\n", name, volume.description);
		}
	}
}

void isbn_free_volume(struct isbn_volume *volume) {
	if (volume->title) {
		free(volume->title);
	}

	if (volume->subtitle) {
		free(volume->subtitle);
	}

	if (volume->author) {
		free(volume->author);
	}

	if (volume->publisher) {
		free(volume->publisher);
	}

	if (volume->publishedDate) {
		free(volume->publishedDate);
	}

	if (volume->description) {
		free(volume->description);
	}
}

static size_t isbn_write(void *ptr, size_t size, size_t nmemb, char *res) {
	size_t real_size = size * nmemb;
	struct isbn_response *mem = (struct isbn_response *)res;

	mem->memory = realloc(mem->memory, mem->size + real_size + 1);

	if (mem->memory == NULL) {
		printf("Memory error\n");

		return 0;
	}

	memcpy(&(mem->memory[mem->size]), ptr, real_size);
	mem->size += real_size;
	mem->memory[mem->size] = (char)0;

	return real_size;
}

void isbn_check_10(char *isbn, int *digit) {
	int i = 0, l = 0, sum = 0;

	for (i = 0, l = 9; i < l; i++) {
		sum += ((int)isbn[i] - 48) * (10 - i);
	}

	*digit = (11 - (sum % 11)) % 11;
}

void isbn_validate_10(char *isbn, int *valid) {
	int i = 0, l = 0, sum = 0;

	for (i = 0, l = 10; i < l; i++) {
		sum += ((int)isbn[i] - 48) * (10 -i);
	}

	*valid = sum % 11 == 0 ? 1 : 0;
}

void isbn_check_13(char *isbn, int *digit) {
	int i = 0, l = 0, sum = 0;

	for (i = 0, l = 12; i < l; i++) {
		sum += ((int)isbn[i] - 48) * (i % 2 == 0 ? 1 : 3);
	}

	*digit = (10 - (sum % 10)) % 10;
}

void isbn_validate_13(char *isbn, int *valid) {
	int i = 0, l = 0, sum = 0;

	for (i = 0, l = 13; i < l; i++) {
		sum += ((int)isbn[i] - 48) * (i % 2 == 0 ? 1 : 3);
	}

	*valid = sum % 10 == 0 ? 1 : 0;
}

int isbn_check(char *isbn, int *digit) {
	int rc = 0;
	size_t length = 0;

	isbn_remove_hyphens(&isbn);

	length = strlen(isbn);

	switch (length) {
	case 9:
		isbn_check_10(isbn, &*digit);
		break;
	case 10:
		isbn[9] = '\0';
		isbn_check_10(isbn, &*digit);
		break;
	case 12:
		isbn_check_13(isbn, &*digit);
		break;
	case 13:
		isbn[12] = '\0';
		isbn_check_13(isbn, &*digit);
		break;
	default:
		printf("Invalid length %zu\n", length);

		rc = 1;
		goto cleanup;
		break;
	}

cleanup:
	return rc;
}

int isbn_validate(char *isbn, int *valid) {
	int rc = 0;
	size_t length = 0;

	isbn_remove_hyphens(&isbn);

	length = strlen(isbn);

	if (length == 10) {
		isbn_validate_10(isbn, &*valid);
	} else if (length == 13) {
		isbn_validate_13(isbn, &*valid);
	} else {
		printf("Invalid length %zu\n", length);

		rc = 1;
		goto cleanup;
	}

cleanup:
	return rc;
}

void isbn_add_hyphens(char **isbn, char *hyphen_input) {
	size_t length = 0, hyphen_length = 0;
	char *tmp = NULL, *chr = NULL, *hyphen = NULL;
	char *identifier = NULL, *identifier_code = NULL;
	char *publisher = NULL, *publisher_code = NULL;
	char *book = NULL;

	if (!hyphen_input) {
		hyphen = malloc(sizeof(char));
		strncpy(hyphen, "-", 1);
	} else {
		hyphen = malloc(sizeof(char) * strlen(hyphen_input) + 1);
		strncpy(hyphen, hyphen_input, strlen(hyphen_input) + 1);
	}

	isbn_remove_hyphens(&*isbn);

	length = strlen(*isbn);
	hyphen_length = strlen(hyphen);

	tmp = malloc(sizeof(char) * (length + (hyphen_length * 4)) + 1);
	chr = malloc(sizeof(char));

    tmp[0] = '\0';

#ifndef _META

    int i = 0, l = 0;
    int hyphen_choice = 0, hyphen_counter = 0, hyphens[2][4] = {
        { 0, 3, 8, 11 },
        { 2, 3, 6, 11 }
    };

    if (length == 13) {
        hyphen_choice = 1;
    }

    for (i = 0, l = (int)length; i < l; i++) {
        strncpy(chr, *isbn + i, 1);
        strcat(tmp, chr);

        if (i == hyphens[hyphen_choice][hyphen_counter]) {
            strcat(tmp, hyphen);
            hyphen_counter++;
        }
    }

#else /* _META */

    int check = 0;
    size_t offset = 0;

	if (length == 13) {
		strcpy(tmp, "978");
		strcat(tmp, hyphen);
		tmp[4] = '\0';

		offset += 3;
	}

	isbn_identifier(&identifier, &identifier_code, &*isbn);
	isbn_publisher(&publisher, &publisher_code, &*isbn);

	if (length == 13) {
		isbn_to_13(&*isbn);
	}

	offset += strlen(identifier_code) + strlen(publisher_code);

	book = malloc(sizeof(char) * 13 + 1);
	book[0] = '\0';

	strcpy(book, *isbn + offset);
	book[strlen(book) - 1] = '\0';

	isbn_check(*isbn, &check);

	sprintf(tmp, "%s%s-%s-%s-%d", tmp, identifier_code, publisher_code, book, check);

#endif /* _META */

	free(*isbn);
	*isbn = malloc(sizeof(char) * (length + (hyphen_length * 4)) + 1);

	strncpy(*isbn, tmp, (length + (hyphen_length * 4)) + 1);

	if (tmp) {
		free(tmp);
	}

	if (chr) {
		free(chr);
	}

	if (hyphen) {
		free(hyphen);
	}

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

	if (book) {
		free(book);
	}
}

void isbn_remove_hyphens(char **isbn) {
	int i = 0, l = 0;
	size_t length = 0;
	char *tmp = NULL, *chr = NULL;

	length = strlen(*isbn);
	tmp = malloc(sizeof(char) * 17 + 1);
	chr = malloc(sizeof(char));

	tmp[0] = '\0';

	for (i = 0, l = (int)length; i < l; i++) {
		strncpy(chr, *isbn + i, 1);

		if ('0' <= chr[0] && chr[0] <= '9') {
			strncat(tmp, chr, 1);
		}
	}

	strncpy(*isbn, tmp, strlen(tmp) + 1);

	if (tmp) {
		free(tmp);
	}

	if (chr) {
		free(chr);
	}
}

void isbn_to_10(char **isbn) {
	int check = 0;
	size_t length = 0;
	char *tmp = NULL;

	length = strlen(*isbn);

	if (length == 10) {
		goto cleanup;
	}

	tmp = malloc(sizeof(char) * 10 + 1);

	strcpy(tmp, "");
	strncat(tmp, *isbn + 3, 9);
	isbn_check_10(tmp, &check);
	(void)snprintf(tmp + 9, 2, "%d", check);

	free(*isbn);
	*isbn = malloc(sizeof(char) * 10 + 1);
	strcpy(*isbn, tmp);

cleanup:
	if (tmp) {
		free(tmp);
	}
}

void isbn_to_13(char **isbn) {
	int check = 0;
	size_t length = 0;
	char *tmp = NULL;

	length = strlen(*isbn);

	if (length == 13) {
		goto cleanup;
	}

	tmp = malloc(sizeof(char) * 13 + 1);

	strcpy(tmp, "978");
	strncat(tmp, *isbn, 9);
	isbn_check_13(tmp, &check);
	(void)snprintf(tmp + 12, 2, "%d", check);

	free(*isbn);
	*isbn = malloc(sizeof(char) * 13 + 1);
	strcpy(*isbn, tmp);

cleanup:
	if (tmp) {
		free(tmp);
	}
}

int isbn_translate(char **isbn) {
	int rc = 0;
	size_t length = 0;

	isbn_remove_hyphens(&*isbn);
	length = strlen(*isbn);

	if (length == 10) {
		isbn_to_13(&*isbn);
	} else if (length == 13) {
		isbn_to_10(&*isbn);
	} else {
		printf("Invalid length %zu\n", length);

		rc = 1;
		goto cleanup;
	}

cleanup:
	return rc;
}

void isbn_fix_hyphens(char **isbn, char *hyphen_input) {
	isbn_remove_hyphens(&*isbn);
	isbn_add_hyphens(&*isbn, hyphen_input);
}

int isbn_curl(char **json, char *url, char *cache_name) {
	int rc = 0;
	size_t cache_size = 0, cache_length = 0;
	char *cache = NULL, *cache_filename = NULL;
	struct isbn_response response;
	FILE *cache_file = NULL;
	CURL *curl;
	CURLcode res;

	response.memory = malloc(1);
	response.size = 0;

#ifndef _CACHE
	cache_name = NULL;
	printf("CACHE OFF\n");
#endif /* _CACHE */

	if (cache_name) {
		cache_length = strlen(isbn_cache) + strlen(cache_name) + 6;
		cache_filename = malloc(sizeof(char) * cache_length + 1);
		(void)snprintf(cache_filename, cache_length, "%s%s%s", isbn_cache, cache_name, ".json");

		cache_file = fopen(cache_filename, "rb");

		if (!cache_file) {
			goto curl;
		}

		(void)fseek(cache_file, 0, SEEK_END);
		cache_size = (size_t)ftell(cache_file);
		(void)fseek(cache_file, 0, SEEK_SET);

		if (cache_size == 0) {
			goto curl;
		}

		cache = malloc(sizeof(char) * cache_size + 1);

		if (!cache) {
			goto curl;
		}

		if (fread(cache, 1, cache_size, cache_file) != cache_size) {
			goto curl;
		}

		*json = malloc(sizeof(char) * cache_size + 1);
		memcpy(*json + cache_size - 1, "\0", 100);
		memcpy(*json, cache, cache_size);
	}

curl:
	if (!*json) {
		curl = curl_easy_init();

		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, isbn_write);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

			res = curl_easy_perform(curl);

			if (res != CURLE_OK) {
				printf("Error: %s\n", curl_easy_strerror(res));

				rc = 1;
				goto cleanup;
			}

			*json = malloc(sizeof(char) * response.size + 1);
			strcpy(*json, response.memory);

			curl_easy_cleanup(curl);
		}

		if (cache == NULL && cache_name != NULL) {
			mkdir(isbn_cache, S_IRWXU);

			cache_file = fopen(cache_filename, "w+");

			if (!cache_file) {
				goto cleanup;
			}

			if (fwrite(*json, 1, response.size, cache_file) != response.size) {
				goto cleanup;
			}
		}
	}

cleanup:
	if (response.memory) {
		free(response.memory);
	}

	if (cache_file) {
		(void)fclose(cache_file);
	}

	if (cache_filename) {
		free(cache_filename);
	}

	if (cache) {
		free(cache);
	}

	curl_global_cleanup();

	return rc;
}

void isbn_string_by_key(char **dest, json_t *src, char *key) {
	size_t length = 0;
	const char *string = NULL;
	json_t *data;

	data = json_object_get(src, key);

	if (!json_is_string(data)) {
		*dest = NULL;
	} else {
		string = json_string_value(data);
		length = strlen(string);

		*dest = malloc(sizeof(char) * length + 1);
		strncpy(*dest, string, length + 1);
	}
}

void isbn_integer_by_key(int *dest, json_t *src, char *key) {
	json_t *data;

	data = json_object_get(src, key);

	if (!json_is_integer(data)) {
		*dest = -1;
	} else {
		*dest = (int)json_integer_value(data);
	}
}

int isbn_lookup(char *isbn, struct isbn_volume *volume) {
	int rc = 0;
	size_t length = 0;
	char *json = NULL, *url = NULL;
	const char *string = NULL;
	json_t *root, *items, *item, *volumeInfo, *authors, *author;
	json_error_t error;

	isbn_remove_hyphens(&isbn);

	length = strlen(isbn_api) + strlen(isbn);
	url = malloc(sizeof(char) * length + 1);

	snprintf(url, length + 1, "%s%s", isbn_api, isbn);

	if (isbn_curl(&json, url, isbn) != 0) {
		rc = 1;
		goto cleanup;
	}

	root = json_loads(json, 0, &error);

	if (!root) {
		printf("Error: on line %d: %s\n", error.line, error.text);

		rc = 1;
		goto cleanup;
	}

	if (!json_is_object(root)) {
		printf("Error: root is not an object\n");

		rc = 1;
		goto cleanup;
	}

	items = json_object_get(root, "items");

	if (!json_is_array(items)) {
		printf("Error: items is not an array\n");

		rc = 1;
		goto cleanup;
	}

	item = json_array_get(items, 0);

	if (!json_is_object(item)) {
		printf("Error: item is not an object\n");

		rc = 1;
		goto cleanup;
	}

	volumeInfo = json_object_get(item, "volumeInfo");

	if (!json_is_object(volumeInfo)) {
		printf("Error: volumeInfo is not an object\n");

		rc = 1;
		goto cleanup;
	}

	isbn_string_by_key(&volume->title, volumeInfo, "title");
	isbn_string_by_key(&volume->subtitle, volumeInfo, "subtitle");
	isbn_string_by_key(&volume->publisher, volumeInfo, "publisher");
	isbn_string_by_key(&volume->publishedDate, volumeInfo, "publishedDate");
	isbn_string_by_key(&volume->description, volumeInfo, "description");
	isbn_integer_by_key(&volume->pageCount, volumeInfo, "pageCount");

	authors = json_object_get(volumeInfo, "authors");

	if (!json_is_array(authors)) {
		printf("Error: authors is not an array\n");

		rc = 1;
		goto cleanup;
	}

	author = json_array_get(authors, 0);

	if (!json_is_string(author)) {
		printf("Error: author is not a string\n");

		rc = 1;
		goto cleanup;
	}

	string = json_string_value(author);
	length = strlen(string);

	volume->author = malloc(sizeof(char) * length + 1);
	strncpy(volume->author, string, length + 1);

cleanup:
	if (url) {
		free(url);
	}

	if (json) {
		free(json);
	}

	return rc;
}