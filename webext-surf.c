#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <gio/gio.h>
#include <gio/gunixfdlist.h>
#include <webkit2/webkit-web-extension.h>
#include <webkitdom/webkitdom.h>
#include <webkitdom/WebKitDOMDOMWindowUnstable.h>

#include "common.h"

#define LENGTH(x)   (sizeof(x) / sizeof(x[0]))

static WebKitWebExtension *webext;
static int sock;

/*
 * Return:
 * 0 No data processed: need more data
 * > 0 Amount of data processed or discarded
 */
static size_t
evalmsg(char *msg, size_t sz)
{
	char js[48];
	WebKitWebPage *page;
	JSCContext *jsc;
	JSCValue *jsv;

	if (!(page = webkit_web_extension_get_page(webext, msg[0])))
		return sz;

	if (sz < 2)
		return 0;

	jsc = webkit_frame_get_js_context(webkit_web_page_get_main_frame(page));
	jsv = NULL;

	switch (msg[1]) {
	case 'h':
		if (sz < 3) {
			sz = 0;
			break;
		}
		sz = 3;
		snprintf(js, sizeof(js),
		         "window.scrollBy(window.innerWidth/100*%hhd,0);",
		         msg[2]);
		jsv = jsc_context_evaluate(jsc, js, -1);
		break;
	case 'v':
		if (sz < 3) {
			sz = 0;
			break;
		}
		sz = 3;
		snprintf(js, sizeof(js),
		         "window.scrollBy(0,window.innerHeight/100*%hhd);",
		         msg[2]);
		jsv = jsc_context_evaluate(jsc, js, -1);
		break;
	default:
		fprintf(stderr, "%s:%d:evalmsg: unknown cmd(%zu): '%#x'\n",
		        __FILE__, __LINE__, sz, msg[1]);
	}

	g_object_unref(jsc);
	if (jsv)
		g_object_unref(jsv);

	return sz;
}

static gboolean
readsock(GIOChannel *s, GIOCondition c, gpointer unused)
{
	static char msg[MSGBUFSZ];
	static size_t msgoff;
	GError *gerr = NULL;
	size_t sz;
	gsize rsz;

	if (g_io_channel_read_chars(s, msg+msgoff, sizeof(msg)-msgoff, &rsz, &gerr) !=
	    G_IO_STATUS_NORMAL) {
		if (gerr) {
			fprintf(stderr, "webext: error reading socket: %s\n",
			        gerr->message);
			g_error_free(gerr);
		}
		return TRUE;
	}
	if (msgoff >= sizeof(msg)) {
		fprintf(stderr, "%s:%d:%s: msgoff: %zu", __FILE__, __LINE__, __func__, msgoff);
		return FALSE;
	}

	for (rsz += msgoff; rsz; rsz -= sz) {
		sz = evalmsg(msg, rsz);
		if (sz == 0) {
			/* need more data */
			break;
		}
		if (sz != rsz) {
			/* continue processing message */
			memmove(msg, msg+sz, rsz-sz);
		}
	}
	msgoff = rsz;

	return TRUE;
}

static void
pageusermessagereply(GObject *o, GAsyncResult *r, gpointer page)
{
	WebKitUserMessage *m;
	GUnixFDList *gfd;
	GIOChannel *gchansock;
	const char *name;
	int nfd;

	m = webkit_web_page_send_message_to_view_finish(page, r, NULL);
	name = webkit_user_message_get_name(m);
	if (strcmp(name, "surf-pipe") != 0) {
		fprintf(stderr, "webext-surf: Unknown User Reply: %s\n", name);
		return;
	}

	gfd = webkit_user_message_get_fd_list(m);
	if ((nfd = g_unix_fd_list_get_length(gfd)) != 1) {
		fprintf(stderr, "webext-surf: Too many file-descriptors: %d\n", nfd);
		return;
	}

	sock = g_unix_fd_list_get(gfd, 0, NULL);

	gchansock = g_io_channel_unix_new(sock);
	g_io_channel_set_encoding(gchansock, NULL, NULL);
	g_io_channel_set_flags(gchansock, g_io_channel_get_flags(gchansock)
	                       | G_IO_FLAG_NONBLOCK, NULL);
	g_io_channel_set_close_on_unref(gchansock, TRUE);
	g_io_add_watch(gchansock, G_IO_IN, readsock, NULL);
}

void
pagecreated(WebKitWebExtension *e, WebKitWebPage *p, gpointer unused)
{
	WebKitUserMessage *msg;

	msg = webkit_user_message_new("page-created", NULL);
	webkit_web_page_send_message_to_view(p, msg, NULL, pageusermessagereply, p);
}

G_MODULE_EXPORT void
webkit_web_extension_initialize(WebKitWebExtension *e)
{
	webext = e;

	g_signal_connect(G_OBJECT(e), "page-created",
	                 G_CALLBACK(pagecreated), NULL);
}
