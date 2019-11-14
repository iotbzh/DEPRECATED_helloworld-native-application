/*
 * Copyright (C) 2015, 2016, 2017 "IoT.bzh"
 * Author José Bollo <jose.bollo@iot.bzh>
 * Author Ronan Le Martret <ronan.lemartret@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <systemd/sd-event.h>
#include <json-c/json.h>

#include <afb/afb-wsj1.h>
#include <afb/afb-ws-client.h>

/* declaration of functions */
static void on_call(void *closure, const char *api, const char *verb, struct afb_wsj1_msg *msg);
static void call(const char *api, const char *verb, const char *object);

/* the callback interface for wsj1 */
static struct afb_wsj1_itf itf = {
	.on_hangup = 0,
	.on_call = on_call,
	.on_event = 0
};

/* global variables */
static struct afb_wsj1 *wsj1;
static sd_event_source *evsrc;
static char *verb = "ping";
static char *data = "true";
static char *api = "helloworld";
static char *logFile="/tmp/helloworld.log";

/* entry function */
int main(int ac, char **av, char **env)
{
	int rc;
	sd_event *loop;
	char uri[500];

	/*get port and token from the command arg*/
	char *port = av[1];
	char *token = av[2];

	/*Generate uri*/
	sprintf (uri,  "127.0.0.1:%s/api?token=%s", port, token);

	/* get the default event loop */
	rc = sd_event_default(&loop);
	if (rc < 0) {
		fprintf(stderr, "connection to default event loop failed: %s\n", strerror(-rc));
		return 1;
	}

	/* connect the websocket wsj1 to the uri given by the first argument */
	wsj1 = afb_ws_client_connect_wsj1(loop, uri, &itf, NULL);
	if (wsj1 == NULL) {
		fprintf(stderr, "connection to %s failed: %m\n", uri);
		return 1;
	}

	/*touch the file log file*/
	int fd = open (logFile, O_RDWR|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	close(fd);

	/* the request*/
	call(api, verb, data);

	/* loop until end */
	for(;;)
		sd_event_run(loop, 30000000);
	return 0;
}

/* called when wsj1 receives a method invocation */
static void on_call(void *closure, const char *api, const char *verb, struct afb_wsj1_msg *msg)
{
	int rc;
	printf("ON-CALL %s/%s:\n%s\n", api, verb,
			json_object_to_json_string_ext(afb_wsj1_msg_object_j(msg),
						JSON_C_TO_STRING_PRETTY));
	fflush(stdout);
	rc = afb_wsj1_reply_error_s(msg, "\"unimplemented\"", NULL);
	if (rc < 0)
		fprintf(stderr, "replying failed: %m\n");
}

/* called when wsj1 receives a reply */
static void on_reply(void *closure, struct afb_wsj1_msg *msg)
{
	const char *result = json_object_to_json_string_ext(afb_wsj1_msg_object_j(msg), JSON_C_TO_STRING_PRETTY);
	/*print json result*/
	printf("ON-REPLY %s: %s\n%s\n", (char*)closure,
			afb_wsj1_msg_is_reply_ok(msg) ? "OK" : "ERROR",
			result);
	fflush(stdout);
	free(closure);
	/*if log file still exist write json result else exit*/
	if ( access( logFile, F_OK ) != -1 )
	{
		FILE* fd = fopen(logFile, "a");
		fprintf(fd, result);
		fclose(fd);
		/*wait before new call*/
		sleep(10);
		call(api, verb, data);
	}
	else
	{
		printf("EXIT AFTER REPLY");
		exit(0);
	}
}

/* makes a call */
static void call(const char *api, const char *verb, const char *object)
{
	static int num = 0;
	char *key;
	int rc;

	/* allocates an id for the request */
	rc = asprintf(&key, "%d:%s/%s", ++num, api, verb);

	/* send the request */
	rc = afb_wsj1_call_s(wsj1, api, verb, object, on_reply, key);
	if (rc < 0) {
		fprintf(stderr, "calling %s/%s(%s) failed: %m\n", api, verb, object);
	}
}
