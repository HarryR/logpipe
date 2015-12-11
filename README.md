# LogPipe

[![Build Status](https://drone.io/github.com/HarryR/apacheclf2json/status.png)](https://drone.io/github.com/HarryR/apacheclf2json/latest)

The logfile pipeline allows you to convert Apache Combined Log files
and quickly convert them into Logstash or Hyperstats format.

The pipeline consists of a number of steps, these are specified
on the commandline, e.g.:

	> logpipe stdin parse.apacheclf \
					 print.logstash stdout

The speed of each step can be measured using the `pv` utlilty:

	$ gzcat test/* | pv -ab | ./bin/logpipe stdin
	9.25MiB [ 346MiB/s]

	$ gzcat test/* | pv -ab |./bin/logpipe stdin parse.apacheclf
	9.25MiB [15.9MiB/s]

	$ gzcat test/* | pv -ab |./bin/logpipe stdin parse.apacheclf print.logstash
	9.25MiB [5.55MiB/s]

It is easy to program custom pipeline modlues, the interface is simple:

	static int init(void **ctx, str_t *str, logline_t *line) {
		*ctx = malloc(...);
		return 1;
	}
	static int test(void *ctx, str_t *str, logline_t *line) {
		return 1;
	}
	const logmod_t mod_name = {
		"name", &init, &test, &test
	};

Then add the module to "src/main.c"

Each module has a name, an init function which takes a pointer to the place that it should create its context pointer, then the run and shutdown functions which are given the context pointer directly.

If a function returns 0 then the pipeline will halt there.