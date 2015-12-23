#ifndef LOGPIPE_H__
#define LOGPIPE_H__

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
 */
logpipe_t *logpipe_new(void);

/**
 * Free all resources and the pipe its self
 */
void logpipe_destroy(logpipe_t *pipe);

/**
 * Execute the pipeline until error
 * @return 
 */
int logpipe_run(logpipe_t *pipe);

/**
 * Execute a pipeline step
 * @return Number of steps remaining
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
int logpipe_steps_add(logpipe_t *pipe, const char *format);

/**
 * Remove a step from the logpipe
 * @param step index to remove
 * @return step count after adding, zero or negative on error
 */
int logpipe_steps_del(logpipe_t *pipe, int step);

/**
 * @return Number of steps in the pipeline
 */
int logpipe_steps_count(const logpipe_t *pipe);

/**
 * Retrieve the current buffer and it's length
 */
const char *logpipe_buf_get(logpipe_t *pipe, int *len);

/**
 * Overwrite the buffer with a new string of a given length
 */
void logpipe_buf_set(logpipe_t *pipe, const char *str, int len);


#endif
