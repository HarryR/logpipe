#ifndef LOGPIPE_H__
#define LOGPIPE_H__

#include <stddef.h>

/* Opaque pointer */
struct logpipe;
typedef struct logpipe logpipe_t;

typedef enum {
	LOGPIPE_TIMESTAMP,	// timestamp
	LOGPIPE_C_IP,		// client_ip, c-ip
	LOGPIPE_CS_IDENT,	// client_identity
	LOGPIPE_CS_USERNAME,// client_auth, cs-username
	LOGPIPE_CS_METHOD,	// req_verb, cs-method
	LOGPIPE_CS_URI_STEM,	// req_path, cs-uri-stem
	LOGPIPE_CS_HTTP_VERSION, // req_ver
	LOGPIPE_SC_CACHE,   // resp_cache, x-edge-result-type
	LOGPIPE_SC_STATUS,  // resp_status, sc-status
	LOGPIPE_BYTES,      // resp_size, bytes, sc-bytes?
	LOGPIPE_CS_REFERER,  // req_referrer, cs(Referer)
	LOGPIPE_CS_USER_AGENT, // req_agent, cs(User-Agent)
	LOGPIPE_TIME_TAKEN,  // duration, time-taken
	LOGPIPE_HIER_CODE,  // hier_code
	LOGPIPE_SC_CONTENT_TYPE, // mime_type, sc(Content-Type)
	LOGPIPE_CS_COOKIE,	// cs(Cookie)
	LOGPIPE_URI_QUERY,	// cs-uri-query
	LOGPIPE_CS_HOST,	// x-host-header, cs(Host)
	// x-edge-location
	// date
	// x-edge-request-id
	// cs-protocol    (http/https)
	// cs-bytes
	// x-forwarded-for
	// ssl-protocol
	// ssl-cipher
	// x-edge-response-result-type???
	LOGPIPE_FIELDS_END,
} logpipe_field_t;

/**
 * Create a new empty pipeline
 * @return pointer to logpipe context
 * @see logpipe_destroy
 */
logpipe_t *logpipe_new(void);

/**
 * Free all resources and the pipe its self
 */
void logpipe_destroy(logpipe_t *pipe);

/**
 * Execute the pipeline until error
 * Upon a successful run, the return value == logpipe_steps_count(..)
 * If the pipeline must halt permanently it will return -1
 * @return returns index of last step which was run 
 */
int logpipe_run(logpipe_t *pipe);

/**
 * Execute a pipeline step
 * @return negative if pipeline must permanently stop,
 * @return zero if pipeline cannot move forward
 */
int logpipe_step(logpipe_t *pipe);

/**
 * Execute the pipeline again & again forever
 * @returns non-zero if terminated early.
 */
int logpipe_run_forever(logpipe_t *pipe);

/**
 * Interrupt the logpipe at the next step
 */
void logpipe_stop(logpipe_t *pipe);

/**
 * Restart the pipeline, going back to step 0
 */
void logpipe_restart(logpipe_t *pipe);

/**
 * Add a step to the logpipe
 * @param format describes the operation to be performed
 * @return step count after adding, zero or negative on error
 */
size_t logpipe_steps_add(logpipe_t *pipe, const char *format);

/**
 * @return Number of steps in the pipeline
 */
size_t logpipe_steps_count(const logpipe_t *pipe);

/**
 * Verify there are steps using logpipe_steps_count
 * before trusting the result of this function.
 * The index will be 0 even if there are no steps.
 * @return Offset of the current step
 */
size_t logpipe_steps_index(const logpipe_t *pipe);

/**
 * Retrieve the current buffer and it's length
 */
const char *logpipe_buf_get(logpipe_t *pipe, size_t *len);

/**
 * Overwrite the buffer with a new string of a given length
 */
void logpipe_buf_set(logpipe_t *pipe, const char *str, size_t len);

/**
 * Create a new pipeline, and verify that it executes correctly
 *
 * The input string may be ommitted by providing NULL, this is
 * equivalent to the default state (no value in buffer).
 *
 * @param result What logpipe_run should return
 * @param steps String containing whitespace separated steps
 * @param input String to use for buffer input
 * @param output Expected string that will be in buffer after execution
 * @return 0 on failure
 */
int logpipe_test(int result, const char *steps, const char *input, const char *output);


#endif
