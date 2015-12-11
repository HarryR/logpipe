#!/bin/bash
( for _ in {1..1000}; do cat test/*.log; done
) | zzuf -r 0.01 -i ./bin/logpipe \
	stdin parse.apacheclf print.logstash stdout \
2> /dev/null | php test/parse.php