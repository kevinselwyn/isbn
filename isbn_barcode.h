#ifndef ISBN_BARCODE_H
#define ISBN_BARCODE_H

extern void isbn_barcode_digit(char **digit, int code, int length);
extern void isbn_barcode_digits(char **barcode, char **isbn);
extern int isbn_barcode_write(char *barcode, char *isbn, char *filename);
extern int isbn_barcode(char **isbn, char *filename);

#endif /* ISBN_BARCODE_H */