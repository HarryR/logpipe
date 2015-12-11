#!/bin/bash
cat test/*.log \
| ./bin/logpipe \
	stdin parse.apacheclf \
	print.clfjson stdout \
	2> /dev/null \
| python test/parse.py 2>> test/python.clfjson \
| php test/parse.php 2>> test/php.clfjson \
; true

cat test/*.log \
| ./bin/logpipe \
	stdin parse.apacheclf \
	print.logstash stdout \
| python test/parse.py 2>> test/python.logstash \
| php test/parse.php 2>> test/php.logstash \
; true

cat test/*.log \
| zzuf -r 0.01 -i \
  ./bin/logpipe \
	stdin parse.apacheclf \
	print.logstash stdout \
	2> /dev/null \
| python test/parse.py 2>> test/python.logstash \
| php test/parse.php 2>> test/php.logstash \
; true