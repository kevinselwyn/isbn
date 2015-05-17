#ifndef ISBN_H
#define ISBN_H

#include <jansson.h>

struct isbn_response {
	char *memory;
	size_t size;
};

struct isbn_volume {
	int pageCount;
	char *title, *subtitle, *author, *publisher;
	char *publishedDate, *description;
};

extern void isbn_dump_volume(struct isbn_volume volume, char *lookup);
extern void isbn_free_volume(struct isbn_volume *volume);
extern void isbn_check_10(char *isbn, int *digit);
extern void isbn_validate_10(char *isbn, int *valid);
extern void isbn_check_13(char *isbn, int *digit);
extern void isbn_validate_13(char *isbn, int *valid);
extern int isbn_check(char *isbn, int *digit);
extern int isbn_validate(char *isbn, int *valid);
extern void isbn_remove_hyphens(char **isbn);
extern void isbn_to_10(char **isbn);
extern void isbn_to_13(char **isbn);
extern int isbn_translate(char **isbn);
extern void isbn_add_hyphens(char **isbn, char *hyphen_input);
extern void isbn_fix_hyphens(char **isbn, char *hyphen_input);
extern int isbn_curl(char **json, char *url, char *cache_name);
extern void isbn_string_by_key(char **dest, json_t *src, char *key);
extern void isbn_integer_by_key(int *dest, json_t *src, char *key);
extern int isbn_lookup(char *isbn, struct isbn_volume *volume);

#endif /* ISBN_H */