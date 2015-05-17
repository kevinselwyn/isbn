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

#define CHECK      0x001
#define VALIDATE   0x002
#define REMOVE     0x004
#define ADD        0x008
#define FIX        0x010
#define TRANSLATE  0x020
#define LOOKUP     0x040
#define IDENTIFIER 0x080
#define PUBLISHER  0x100
#define BOOK       0x200
#define EAN        0x400

static void usage(char *exec) {
	int length = (int)strlen(exec) + 1;

	printf("%s [-c|--check] [-v|--validate] [-r|--remove] [-a|--add] [-f|--fix]\n", exec);
#ifdef _META
	printf("%*s[-t|--translate] [-l|--lookup <info>] [-i|--identifier]\n", length, " ");
#ifdef _BARCODE
	printf("%*s[-p|--publisher] [-b|--book] [-e|--ean <filename>]\n", length, " ");
#else /* _BARCODE */
	printf("%*s[-p|--publisher] [-b|--book]\n", length, " ");
#endif /* _BARCODE */
#else /* _META */
#ifdef _BARCODE
	printf("%*s[-t|--translate] [-l|--lookup <info>] [-e|--ean <filename>]\n", length, " ");
#else /* _BARCODE */
	printf("%*s[-t|--translate] [-l|--lookup <info>]\n", length, " ");
#endif /* _BARCODE */
#endif /* _META */
	printf("%*s[-h|--help]\n", length, " ");
	printf("%*s<isbn>\n", length, " ");
}

int main(int argc, char *argv[]) {
	int rc = 0, i = 0, l = 0, flags = 0, response = 0;
	char *exec = NULL, *isbn = NULL, *action = NULL, *hyphen = NULL;
	char *lookup = NULL, *identifier = NULL, *identifier_code = NULL;
	char *publisher = NULL, *publisher_code = NULL, *book = NULL;
	struct isbn_volume volume;

    #ifdef _BARCODE
    char *ean_filename = NULL;
    #endif /* _BARCODE */

	exec = argv[0];

	if (argc < 2) {
		usage(exec);

		rc = 1;
		goto cleanup;
	}

	isbn = malloc(sizeof(char) * 17 + 1);
	isbn[0] = '\0';

	for (i = 1, l = argc; i < l; i++) {
		action = argv[i];

		if (strncmp(action, "-", 1) == 0) {
			if (strncmp(action, "-c", 2) == 0 || strncmp(action, "--check", 7) == 0) {
				flags += CHECK;
			} else if (strncmp(action, "-v", 2) == 0 || strncmp(action, "--validate", 10) == 0) {
				flags += VALIDATE;
			} else if (strncmp(action, "-r", 2) == 0 || strncmp(action, "--remove", 8) == 0) {
				flags += REMOVE;
			} else if (strncmp(action, "-a", 2) == 0 || strncmp(action, "--add", 5) == 0) {
				flags += ADD;

				if (strncmp(argv[i + 1], "-", 1) != 0 && i + 2 < argc) {
					hyphen = argv[++i];
				}
			} else if (strncmp(action, "-f", 2) == 0 || strncmp(action, "--fix", 5) == 0) {
				flags += FIX;

				if (strncmp(argv[i + 1], "-", 1) != 0 && i + 2 < argc) {
					hyphen = argv[++i];
				}
			} else if (strncmp(action, "-t", 2) == 0 || strncmp(action, "--translate", 11) == 0) {
				flags += TRANSLATE;
			} else if (strncmp(action, "-l", 2) == 0 || strncmp(action, "--lookup", 8) == 0) {
				flags += LOOKUP;

				if (strncmp(argv[i + 1], "-", 1) != 0 && i + 2 < argc) {
					lookup = argv[++i];
				}
#ifdef _META
			} else if (strncmp(action, "-i", 2) == 0 || strncmp(action, "--identifier", 12) == 0) {
				flags += IDENTIFIER;
			} else if (strncmp(action, "-p", 2) == 0 || strncmp(action, "--publisher", 11) == 0) {
				flags += PUBLISHER;
			} else if (strncmp(action, "-b", 2) == 0 || strncmp(action, "--book", 6) == 0) {
				flags += BOOK;
#endif /* _META */
#ifdef _BARCODE
			} else if (strncmp(action, "-e", 2) == 0 || strncmp(action, "--ean", 5) == 0) {
				flags += EAN;

				if (strncmp(argv[i + 1], "-", 1) != 0 && i + 2 < argc) {
					ean_filename = argv[++i];
				}
#endif /* _BARCODE */
			} else {
				if (strncmp(action, "-h", 2) == 0 || strncmp(action, "--help", 6) == 0) {
					usage(exec);
				} else {
					printf("Invalid action %s\n", action);
				}

				rc = 1;
				goto cleanup;
			}
		} else {
			strcpy(isbn, action);
		}
	}

	if (!isbn) {
		usage(exec);

		rc = 1;
		goto cleanup;
	}

#ifdef _META
	isbn_load_data(&isbn_identifiers, "data/isbn_identifiers.dat");
	isbn_load_data(&isbn_publishers[0], "data/isbn_publishers_0.dat");
	isbn_load_data(&isbn_publishers[1], "data/isbn_publishers_1.dat");
#endif

	if (flags != 0) {
		if ((flags & (CHECK | VALIDATE)) != 0) {
			if ((flags & CHECK) != 0) {
				if (isbn_check(isbn, &response) != 0) {
					rc = 1;
					goto cleanup;
				}
			} else {
				if (isbn_validate(isbn, &response) != 0) {
					rc = 1;
					goto cleanup;
				}
			}

			printf("%d\n", response);
		} else {
			if ((flags & (LOOKUP | IDENTIFIER | PUBLISHER | BOOK)) != 0) {
				if ((flags & LOOKUP) != 0) {
					if (isbn_lookup(isbn, &volume) != 0) {
						rc = 1;
						goto cleanup;
					}

					isbn_dump_volume(volume, lookup);
					isbn_free_volume(&volume);
#ifdef _META
				} else if ((flags & IDENTIFIER) != 0) {
					isbn_identifier(&identifier, &identifier_code, &isbn);

					printf("%s\n", identifier);
				} else if ((flags & PUBLISHER) != 0) {
					isbn_publisher(&publisher, &publisher_code, &isbn);

					printf("%s\n", publisher);
				} else {
					isbn_book(&book, &isbn);

					printf("%s\n", book);
#endif /* _META */
				}
#ifdef _BARCODE
			} else if ((flags & EAN) != 0) {
				if (isbn_barcode(&isbn, ean_filename) != 0) {
					rc = 1;
					goto cleanup;
				}
#endif /* _BARCODE */
			} else {
				if ((flags & REMOVE) != 0) {
					isbn_remove_hyphens(&isbn);
				} else if ((flags & (ADD | FIX)) != 0) {
					if (!hyphen) {
						hyphen = "-";
					}

					if ((flags & ADD) != 0) {
						isbn_add_hyphens(&isbn, hyphen);
					} else {
						isbn_fix_hyphens(&isbn, hyphen);
					}
				} else if ((flags & TRANSLATE) != 0) {
					if (isbn_translate(&isbn) != 0) {
						rc = 1;
						goto cleanup;
					}
				}

				printf("%s\n", isbn);
			}
		}
	} else {
		usage(exec);

		rc = 1;
		goto cleanup;
	}

cleanup:
	if (isbn) {
		free(isbn);
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

#ifdef _META
	isbn_free_data(&isbn_identifiers);
	isbn_free_data(&isbn_publishers[0]);
	isbn_free_data(&isbn_publishers[1]);
#endif

	return rc;
}