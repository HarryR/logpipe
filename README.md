# LogPipe

[![Build Status](https://drone.io/github.com/HarryR/logpipe/status.png)](https://drone.io/github.com/HarryR/logpipe/latest)
[![Build Status](https://semaphoreci.com/api/v1/projects/009f6bc1-43e6-4ab1-8b3a-50cc19cccaa8/633027/badge.svg)](https://semaphoreci.com/harryr/logpipe)
[![Build Status](https://travis-ci.org/HarryR/logpipe.svg)](https://travis-ci.org/HarryR/logpipe)

The logfile pipeline allows you to convert Apache Combined Log files
and quickly convert them into Logstash or Hyperstats format.

The pipeline consists of a number of steps, these are specified
on the commandline, e.g.:

	> logpipe stdin parse.apacheclf \
					 print.logstash stdout

The speed of each step can be measured using the `pv` utlilty:

	# Add line to buffer from stdin
	$ cat test.clf | pv -ab | ./bin/logpipe stdin  &> /dev/null
	12.3MiB [ 588MiB/s]

	# Parse line, 12x slower than NOOP...
	cat test.clf | pv -ab | ./bin/logpipe stdin parse.apacheclf &> /dev/null
	12.3MiB [49.3MiB/s]

	# Print apacheclf, 1.5x slower
	$ cat test.clf | pv -ab | ./bin/logpipe stdin parse.apacheclf print.apacheclf &> /dev/null
	12.3MiB [30.4MiB/s]

	# Print to stdout, 0x slower
	$ cat test.clf | pv -ab | ./bin/logpipe stdin parse.apacheclf print.apacheclf stdout &> /dev/null
	12.3MiB [29.9MiB/s]

The pipeline has a buffer string and line state struct, raw log lines are read
into the buffer by the `stdin` module, then parsed into the `line` struct 
by `parse.apacheclf`, and then the `pring.logstash` module resets the string
buffer and fills it with logstash JSON. This could then be printed using `stdout`.

## Getting Started

    sudo apt-get install zzuf cmake ragel
    cmake .
    make
    ./test.sh

## Commands

 * reset.str - Empty string buffer
 * reset.line - Empty line struct
 * reset - Reset both str and line
 * debug.line - Print line status
 * debug.anon - Anonymize parsed fields
 * debug.randblank - Randomly blank fields
 * stdin - Read line from stdin into buffer
 * stdout - Write buffer to stdout
 * parse.apacheclf - Parse Apache CLF
 * print.apacheclf - Print Apache CLF
 * parse.clfjson - Parse JSON array, CLF
 * print.clfjson - FIll buffer with JSON CLF
 * print.logstash - Fill buffer with logstash JSON
 * print.hyperstats - Fill buffer with hyperstats JSON

## Modules

It is easy to program custom pipeline modlues, the interface is simple:

	#include "logpipe-module.h"
	#include <syslog.h>

	static int run_syslog(void *ctx, str_t *buf, logmeta_t *meta) {
		syslog(LOG_ERR, "%.*s", (int)buf->len, buf->ptr);
		return 1;
	}
	const logmod_t mod_syslog = {
		"syslog", NULL, &run_syslog, NULL
	};

	static int init(void **ctx, str_t *str, logmeta_t *meta) {
		*ctx = malloc(...);
		return 1;
	}
	static int test(void *ctx, str_t *str, logmeta_t *meta) {
		return 1;
	}
	static int shutdown(void *ctx, str_t *str, logmeta_t *meta) {
		free(ctx);
		return 1;
	}
	const logmod_t mod_name = {
		"name", &init, &test, &shutdown
	};

Then define the module in `logpipe-module.h` and rebuild logpipe.

Each module has a name, an init function which takes a pointer to its own context pointer, then the run and shutdown functions which are given the context pointer directly.

An initialisation function must return 0 or less to indicate a failure, if one fails then the step cannot be added to the pipeline - it must free all resources it allocated.

The return value of the shutdown function will be ignored.

If the return value of the execute function is zero the pipeline will stop there and not continue, this is used to indicate that the a non-fatal error has occurred. If the return value of the execute function is below zero this indicates a fatal error has occurred and the whole pipeline should permanently stop. Values above zero are ignored.

The `meta` context, passed as the third argument, enables plugins to manage information extracted from the buffer, for example parsing an Apache Combined Log Format line into its various fields.
