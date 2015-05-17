#ifndef ISBN_META_H
#define ISBN_META_H

struct isbn_data_struct {
	int data_count;
	char ***data;
};

struct isbn_data_struct isbn_identifiers;
struct isbn_data_struct isbn_publishers[2];

extern int isbn_load_data(struct isbn_data_struct *data, char *filename);
extern void isbn_free_data(struct isbn_data_struct *data);
extern void isbn_identifier(char **identifier, char **identifier_code, char **isbn);
extern void isbn_publisher(char **publisher, char **publisher_code, char **isbn);
extern void isbn_book(char **book, char **isbn);

#endif /* ISBN_META_H */