#!/bin/bash

exec=./bin/isbn
true="1"

red="\033[0;31m"
green="\033[0;32m"
yellow="\033[0;33m"
end="\033[0m"

isbn9="034533970"
isbn9_hyphen="0-345-33970"
isbn10="0345339703"
isbn10_check="3"
isbn10_hyphen="0-345-33970-3"
isbn10_fix="0/345/33970/3"

isbn12="978034533970"
isbn12_hyphen="978-0-345-33970"
isbn13="9780345339706"
isbn13_check="6"
isbn13_hyphen="978-0-345-33970-6"
isbn13_fix="978/0/345/33970/6"

isbn_identifier="English"
isbn_publisher="Ballantine Books"
isbn_book="33970"

lookup_title="The Fellowship of the Ring"
lookup_subtitle="The Lord of the Rings--Part One"
lookup_author="John Ronald Reuel Tolkien"
lookup_publisher="Del Rey"
lookup_publishedDate="1994"
lookup_pageCount="480"
lookup_description="THE GREATEST FANTASY EPIC OF OUR TIME The dark, fearsome Ringwraiths were searching for a hobbit. Frodo Baggins knew they were seeking him and the Ring he bore -- the Ring of Power that would enable evil Sauron to destroy all that was good in Middle-earth. Now it is up to Frodo and his faithful servant, Sam, with a small band of companions, to carry the Ring to the one place it could be destroyed -- Mount Doom, in the very center of Sauron's dark kingdom. THUS BEGINS J.R.R. TOLKIEN'S CLASSIC THE LORD OF THE RINGS, WHICH CONTINUES IN THE TWO TOWERS AND THE RETURN OF THE KING."

test()
{
	if [ "$expected" == "$actual" ]
	then
		echo -e "  ${green}PASS${end} $1"
	else
		echo -e "  ${red}FAIL${end} $1"
		echo "    Expected: $expected"
		echo -n "    Actual:   "
		echo "$actual"
	fi
}

warning()
{
	echo -e "  ${yellow}WARNING${end} $1"
	echo "    $2"
}

titles=("Check" "Validate" "Add" "Remove" "Fix" "Translate" "Identifier" "Publisher" "Book")
titles_lookup=("Title" "Subtitle" "Author" "Publisher" "Publish Date" "Page Count" "Description")
expected10=("$isbn10_check" "$true" "$isbn10_hyphen" "$isbn10" "$isbn10_hyphen" "$isbn13" "$isbn_identifier" "$isbn_publisher" "$isbn_book")
expected13=("$isbn13_check" "$true" "$isbn13_hyphen" "$isbn13" "$isbn13_hyphen" "$isbn10" "$isbn_identifier" "$isbn_publisher" "$isbn_book")
expected_lookup=("$lookup_title" "$lookup_subtitle" "$lookup_author" "$lookup_publisher" "$lookup_publishedDate" "$lookup_pageCount" "$lookup_description")
actual10=("$exec -c $isbn9" "$exec -v $isbn10" "$exec -a $isbn10" "$exec -r $isbn10_hyphen" "$exec -f $isbn10_fix" "$exec -t $isbn10" "$exec -i $isbn10" "$exec -p $isbn10" "$exec -b $isbn10")
actual10_hyphen=("$exec -c $isbn9_hyphen" "$exec -v $isbn10_hyphen" "$exec -a $isbn10_hyphen" "$exec -r $isbn10_hyphen" "$exec -f $isbn10_fix" "$exec -t $isbn10_hyphen" "$exec -i $isbn10_hyphen" "$exec -p $isbn10_hyphen" "$exec -b $isbn10_hyphen")
actual10_lookup=("$exec -l title $isbn10" "$exec -l subtitle $isbn10" "$exec -l author $isbn10" "$exec -l publisher $isbn10" "$exec -l publishedDate $isbn10" "$exec -l pageCount $isbn10" "$exec -l description $isbn10")
actual10_hyphen_lookup=("$exec -l title $isbn10_hyphen" "$exec -l subtitle $isbn10_hyphen" "$exec -l author $isbn10_hyphen" "$exec -l publisher $isbn10_hyphen" "$exec -l publishedDate $isbn10_hyphen" "$exec -l pageCount $isbn10_hyphen" "$exec -l description $isbn10_hyphen")
actual13=("$exec -c $isbn12" "$exec -v $isbn13" "$exec -a $isbn13" "$exec -r $isbn13_hyphen" "$exec -f $isbn13_fix" "$exec -t $isbn13" "$exec -i $isbn13" "$exec -p $isbn13" "$exec -b $isbn13")
actual13_hyphen=("$exec -c $isbn12_hyphen" "$exec -v $isbn13_hyphen" "$exec -a $isbn13_hyphen" "$exec -r $isbn13_hyphen" "$exec -f $isbn13_fix" "$exec -t $isbn13_hyphen" "$exec -i $isbn13_hyphen" "$exec -p $isbn13_hyphen" "$exec -b $isbn13_hyphen")
actual13_lookup=("$exec -l title $isbn13" "$exec -l subtitle $isbn13" "$exec -l author $isbn13" "$exec -l publisher $isbn13" "$exec -l publishedDate $isbn13" "$exec -l pageCount $isbn13" "$exec -l description $isbn13")
actual13_hyphen_lookup=("$exec -l title $isbn13_hyphen" "$exec -l subtitle $isbn13_hyphen" "$exec -l author $isbn13_hyphen" "$exec -l publisher $isbn13_hyphen" "$exec -l publishedDate $isbn13_hyphen" "$exec -l pageCount $isbn13_hyphen" "$exec -l description $isbn13_hyphen")

echo "ISBN-10 - $isbn10"

for i in `seq 0 8`
do
	expected=${expected10[i]}
	actual=`eval ${actual10[i]}`
	test "${titles[i]}"
done

echo "ISBN-10 - $isbn10_hyphen"

for i in `seq 0 8`
do
	expected=${expected10[i]}
	actual=`eval ${actual10_hyphen[i]}`
	test "${titles[i]}"
done

echo "Lookup - $isbn10"

for i in `seq 0 6`
do
	expected=${expected_lookup[i]}
	actual=`eval ${actual10_lookup[i]}`
	test "${titles_lookup[i]}"
done

echo "Lookup - $isbn10_hyphen"

for i in `seq 0 6`
do
	expected=${expected_lookup[i]}
	actual=`eval ${actual10_hyphen_lookup[i]}`
	test "${titles_lookup[i]}"
done

echo "ISBN-13 - $isbn13"

for i in `seq 0 8`
do
	expected=${expected13[i]}
	actual=`eval ${actual13[i]}`
	test "${titles[i]}"
done

echo "ISBN-13 - $isbn13_hyphen"

for i in `seq 0 8`
do
	expected=${expected13[i]}
	actual=`eval ${actual13_hyphen[i]}`
	test "${titles[i]}"
done

echo "Lookup - $isbn13"

for i in `seq 0 6`
do
	expected=${expected_lookup[i]}
	actual=`eval ${actual13_lookup[i]}`
	test "${titles_lookup[i]}"
done

echo "Lookup - $isbn13_hyphen"

for i in `seq 0 6`
do
	expected=${expected_lookup[i]}
	actual=`eval ${actual13_hyphen_lookup[i]}`
	test "${titles_lookup[i]}"
done