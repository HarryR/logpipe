#!/bin/bash
INPUT_APACHECLF=test/all.apacheclf
REGRESSION_CLFJSON=test/regression.clfjson

if [ ! -f $INPUT_APACHECLF ]; then
	URLS="http://www.thelyric.com/pics/_/logs/thelyric.com/http.4740641.bak/access.log.2011-08-15
		 http://www.evolveplasticmd.com/evolveplasticmd.com/log/20140428-access.log
		 http://www.sankus.com/log/access.log
		 http://www.swanksalon.info/logs/access.log
		 http://www.amore-restaurant.co.uk/log/access.log"
	for URL in $URLS; do
		echo "Retrieving $URL"
		curl $URL >> $INPUT_APACHECLF
	done
fi

rm -f test/python.* test/php.*

echo Test JSONCLF to self
head -n 10 $INPUT_APACHECLF | valgrind --leak-check=full ./bin/logpipe stdin \
	parse.apacheclf print.apacheclf parse.apacheclf \
	print.clfjson parse.clfjson \
	2> /dev/null 

echo Test clfjson output
cat $INPUT_APACHECLF | ./bin/logpipe \
	stdin parse.apacheclf \
	print.clfjson stdout \
	2> $REGRESSION_CLFJSON \
| python test/parse.py 2> test/python.clfjson \
| php test/parse.php 2> test/php.clfjson

echo Test logstash output
cat $INPUT_APACHECLF | ./bin/logpipe \
	stdin parse.apacheclf \
	print.logstash \
| python test/parse.py 2> test/python.logstash \
| php test/parse.php 2> test/php.logstash

cat $INPUT_APACHECLF | zzuf -i | ./bin/logpipe stdin \
	parse.apacheclf print.apacheclf parse.apacheclf \
	print.clfjson stdout \
	2> /dev/null \
| python test/parse.py 2>> test/python.clfjson \
| php test/parse.php 2>> test/php.clfjson \
> /dev/null

echo Test ensure can parse own output that python and php think is broken
cat test/python.clfjson test/php.clfjson \
| ./bin/logpipe stdin parse.clfjson 2>> $REGRESSION_CLFJSON

echo Anonymize can read its own output
cat test/python.clfjson test/php.clfjson \
| ./bin/logpipe stdin parse.clfjson debug.anon print.clfjson parse.clfjson 2>> $REGRESSION_CLFJSON

echo Check clfjson full-circle under valgrind
head -n 1000 $INPUT_APACHECLF \
| valgrind --leak-check=full ./bin/logpipe \
	stdin parse.apacheclf \
	print.clfjson parse.clfjson \
> /dev/null

echo "clfjson regression rate:"
wc -l $REGRESSION_CLFJSON
echo