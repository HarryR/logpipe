#!/bin/bash
INPUT_APACHECLF=test/all.apacheclf
INPUT_SQUID=test/all.squid
REGRESSION_CLFJSON=test/regression.clfjson

if [ ! -f $INPUT_APACHECLF ]; then
	URLS="http://www.thelyric.com/pics/_/logs/thelyric.com/http.4740641.bak/access.log.2011-08-15
		  http://www.evolveplasticmd.com/evolveplasticmd.com/log/20140428-access.log
		  http://www.sankus.com/log/access.log
		  http://www.swanksalon.info/logs/access.log
		  http://www.amore-restaurant.co.uk/log/access.log"
	for URL in $URLS; do
		echo "Retrieving test ApacheCLF log from: $URL"
		curl -s $URL >> $INPUT_APACHECLF
	done
fi

if [ ! -f $INPUT_SQUID ]; then
	URLS="http://minkirri.apana.org.au/~abo/projects/squid/access.log
		  http://www2.contilnet.com.br/~Curso_Tecnico/notas/access.log
		  http://www.stillhq.com/extracted/sqrad/access.log"
	for URL in $URLS; do
		echo "Retrieving test Squid log from: $URL"
		curl -s $URL >> $INPUT_SQUID
	done
fi

rm -f test/python.* test/php.*

PRINT_FNS="print.apacheclf print.logstash print.hyperstats"
for PRINT_FN in $PRINT_FNS
do
	echo "Test $PRINT_FN with random blanks"
	cat $INPUT_APACHECLF \
	| ./bin/logpipe stdin parse.apacheclf \
					debug.randblank $PRINT_FN \
	2> /dev/null
done

echo Test JSONCLF to self
head -n 10 $INPUT_APACHECLF | ./bin/logpipe stdin \
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

which valgrind
if [ $? -eq 0 ]; then
	echo Check clfjson full-circle under valgrind
	head -n 1000 $INPUT_APACHECLF \
	| valgrind --leak-check=full ./bin/logpipe \
		stdin parse.apacheclf debug.anon \
		print.clfjson parse.clfjson \
	> /dev/null
fi

echo "clfjson regression rate:"
wc -l $REGRESSION_CLFJSON
echo