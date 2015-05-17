#isbn

ISBN utility for validating, calculating check digits, adding/removing/fixing hyphens, and generating barcode images

##Installation

```bash
make && sudo make install
```

To turn on file-based caching, make with:

```bash
make cache && sudo make install
```

(Default:) To include the identifier and publisher metadata (additional ~100K), make with:

```bash
make meta && sudo make install
```

To make with caching and metadata:

```bash
make cache_meta && sudo make install
```

To change the cache directory, change the `isbn_cache` variable in `isbn.h`

##Usage

```
isbn [-c|--check] [-v|--validate] [-r|--remove] [-a|--add] [-f|--fix]
     [-t|--translate] [-l|--lookup <info>] [-i|--identifier]
     [-p|--publisher] [-b|--book] [-e|--ean <filename>]
     [-h|--help]
     <isbn>
```

| Flag                | Description                                                                                           |
| ------------------- | ----------------------------------------------------------------------------------------------------- |
| -c,--check          | Calculates check digit                                                                                |
| -v,--validate       | Validates ISBN (1: valid, 0: invalid)                                                                 |
| -r,--remove         | Removes hyphens                                                                                       |
| -a,--add            | Adds hyphens                                                                                          |
| -f,--fix            | Fixes hyphens                                                                                         |
| -t,--translate      | Translates to 10-digit ISBN to 13-digit and vice versa                                                |
| -l,--lookup <info>  | Looks up book information (title, subtitle, author, publisher, publishedDate, pageCount, description) |
| -i,--identifier     | Returns national/language group based on the ISBN identifier code                                     |
| -p,--publisher      | Returns publisher based on the ISBN publisher code                                                    |
| -b,--book           | Returns book title code                                                                               |
| -e,--ean <filename> | Generates a barcode                                                                                   |
| -h,--help           | Prints program usage                                                                                  |

##Examples

```
isbn -c 978034533970
6

isbn -v 9780345339706
1

isbn -r 978-03-45339-70-6
9780345339706

isbn -a 9780345339706
978-03-45339-70-6

isbn -f 978/03/45339/70/6
978-03-45339-70-6

isbn -t 9780345339706
0345339703

isbn -l 9780345339706
Title:        The Fellowship of the Ring
Subtitle:     The Lord of the Rings--Part One
Author:       John Ronald Reuel Tolkien
Publisher:    Del Rey
Publish Date: 1994
Page Count:   480
Description:
THE GREATEST FANTASY EPIC OF OUR TIME The dark, fearsome Ringwraiths were searching for a hobbit. Frodo Baggins knew they were seeking him and the Ring he bore -- the Ring of Power that would enable evil Sauron to destroy all that was good in Middle-earth. Now it is up to Frodo and his faithful servant, Sam, with a small band of companions, to carry the Ring to the one place it could be destroyed -- Mount Doom, in the very center of Sauron's dark kingdom. THUS BEGINS J.R.R. TOLKIEN'S CLASSIC THE LORD OF THE RINGS, WHICH CONTINUES IN THE TWO TOWERS AND THE RETURN OF THE KING.

isbn -l title 9780345339706
The Fellowship of the Ring

isbn -l subtitle 9780345339706
The Lord of the Rings--Part One

isbn -l author 9780345339706
John Ronald Reuel Tolkien

isbn -l publisher 9780345339706
Del Rey

isbn -l publishedDate 9780345339706
1994

isbn -l pageCount 9780345339706
480

isbn -l description 9780345339706
THE GREATEST FANTASY EPIC OF OUR TIME The dark, fearsome Ringwraiths were searching for a hobbit. Frodo Baggins knew they were seeking him and the Ring he bore -- the Ring of Power that would enable evil Sauron to destroy all that was good in Middle-earth. Now it is up to Frodo and his faithful servant, Sam, with a small band of companions, to carry the Ring to the one place it could be destroyed -- Mount Doom, in the very center of Sauron's dark kingdom. THUS BEGINS J.R.R. TOLKIEN'S CLASSIC THE LORD OF THE RINGS, WHICH CONTINUES IN THE TWO TOWERS AND THE RETURN OF THE KING.

isbn -i 9780345339706
English

isbn -p 9780345339706
Ballantine Books

isbn -b 9780345339706
33970

isbn -e 9780345339706
Generated barcode 9780345339706.png
```

##Additional Information

Read the Wikipedia article on [ISBNs](http://en.wikipedia.org/wiki/International_Standard_Book_Number) for more information about validation and calculating check digits

##Acknowledgements

