/*
 * ws protocol handler plugin for "lws-minimal"
 *
 * Written in 2010-2019 by Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * This version holds a single message at a time, which may be lost if a new
 * message comes.  See the minimal-ws-server-ring sample for the same thing
 * but using an lws_ring ringbuffer to hold up to 8 messages at a time.
 */

#if !defined (LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#endif

#include <string.h>

/* one of these created for each message */

struct msg {
	void *payload; /* is malloc'd */
	size_t len;
	uint64_t timestamp;
	struct msg *next;
};

/* one of these is created for each client connecting to us */

struct per_session_data__minimal {
	struct per_session_data__minimal *pss_list;
	struct lws *wsi;
	int last; /* the last message number we sent */
	int needs_history; /* flag to indicate this client needs history */
	struct msg *history_pos; /* current position in history sending */
};

/* one of these is created for each vhost our protocol is used with */

struct per_vhost_data__minimal {
	struct lws_context *context;
	struct lws_vhost *vhost;
	const struct lws_protocols *protocol;

	struct per_session_data__minimal *pss_list; /* linked-list of live pss*/

	struct msg amsg; /* the one pending message... */
	int current; /* the current message number we are caching */
	
	/* Message history */
	struct msg *message_history_head;
	struct msg *message_history_tail;
	int message_count;
	int max_history_messages;
};

/* destroys the message when everyone has had a copy of it */

static void
__minimal_destroy_message(void *_msg)
{
	struct msg *msg = _msg;

	free(msg->payload);
	msg->payload = NULL;
	msg->len = 0;
}

/* Function to add message to history */
static void
__minimal_add_to_history(struct per_vhost_data__minimal *vhd, void *payload, size_t len)
{
	struct msg *new_msg = malloc(sizeof(struct msg));
	if (!new_msg) return;

	new_msg->payload = malloc(LWS_PRE + len);
	if (!new_msg->payload) {
		free(new_msg);
		return;
	}

	memcpy((char *)new_msg->payload + LWS_PRE, payload, len);
	new_msg->len = len;
	new_msg->next = NULL;

	/* Add to tail of history list */
	if (vhd->message_history_tail) {
		vhd->message_history_tail->next = new_msg;
	} else {
		vhd->message_history_head = new_msg;
	}
	vhd->message_history_tail = new_msg;
	vhd->message_count++;

	/* Remove old messages if we exceed limit */
	while (vhd->message_count > vhd->max_history_messages && vhd->message_history_head) {
		struct msg *old_head = vhd->message_history_head;
		vhd->message_history_head = vhd->message_history_head->next;
		if (!vhd->message_history_head) {
			vhd->message_history_tail = NULL;
		}
		free(old_head->payload);
		free(old_head);
		vhd->message_count--;
	}
}

/* Function to send message history to a new client */
static void
__minimal_send_history(struct per_session_data__minimal *pss, struct per_vhost_data__minimal *vhd)
{
	/* Mark this client as needing history and set starting position */
	pss->needs_history = 1;
	pss->history_pos = vhd->message_history_head;
	lws_callback_on_writable(pss->wsi);
}

static int
callback_minimal(struct lws *wsi, enum lws_callback_reasons reason,
			void *user, void *in, size_t len)
{
	struct per_session_data__minimal *pss =
			(struct per_session_data__minimal *)user;
	struct per_vhost_data__minimal *vhd =
			(struct per_vhost_data__minimal *)
			lws_protocol_vh_priv_get(lws_get_vhost(wsi),
					lws_get_protocol(wsi));
	int m;

	switch (reason) {
	case LWS_CALLBACK_PROTOCOL_INIT:
		vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
				lws_get_protocol(wsi),
				sizeof(struct per_vhost_data__minimal));
		if (!vhd)
			return 1;
		vhd->context = lws_get_context(wsi);
		vhd->protocol = lws_get_protocol(wsi);
		vhd->vhost = lws_get_vhost(wsi);
		
		/* Initialize message history */
		vhd->message_history_head = NULL;
		vhd->message_history_tail = NULL;
		vhd->message_count = 0;
		vhd->max_history_messages = 50; /* Store up to 50 messages */
		break;

	case LWS_CALLBACK_ESTABLISHED:
		/* add ourselves to the list of live pss held in the vhd */
		lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
		pss->wsi = wsi;
		pss->last = vhd->current;
		pss->needs_history = 0;
		pss->history_pos = NULL;
		
		/* Send message history to the new client */
		__minimal_send_history(pss, vhd);
		break;

	case LWS_CALLBACK_CLOSED:
		/* remove our closing pss from the list of live pss */
		lws_ll_fwd_remove(struct per_session_data__minimal, pss_list,
				  pss, vhd->pss_list);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		/* Handle history sending first for new clients */
		if (pss->needs_history && pss->history_pos) {
			/* Send one history message at a time */
			m = lws_write(wsi, ((unsigned char *)pss->history_pos->payload) +
				      LWS_PRE, pss->history_pos->len, LWS_WRITE_TEXT);
			if (m < (int)pss->history_pos->len) {
				lwsl_err("ERROR %d writing history to ws\n", m);
				return -1;
			}
			
			/* Move to next history message */
			pss->history_pos = pss->history_pos->next;
			
			/* If more history to send, request another writable callback */
			if (pss->history_pos) {
				lws_callback_on_writable(wsi);
			} else {
				/* Done sending history */
				pss->needs_history = 0;
				pss->last = vhd->current; /* Catch up to current */
			}
			break;
		}
		
		/* Handle regular new messages */
		if (!vhd->amsg.payload)
			break;

		if (pss->last == vhd->current)
			break;

		/* notice we allowed for LWS_PRE in the payload already */
		m = lws_write(wsi, ((unsigned char *)vhd->amsg.payload) +
			      LWS_PRE, vhd->amsg.len, LWS_WRITE_TEXT);
		if (m < (int)vhd->amsg.len) {
			lwsl_err("ERROR %d writing to ws\n", m);
			return -1;
		}

		pss->last = vhd->current;
		break;

	case LWS_CALLBACK_RECEIVE:
		/* Add message to history before processing */
		__minimal_add_to_history(vhd, in, len);
		
		if (vhd->amsg.payload)
			__minimal_destroy_message(&vhd->amsg);

		vhd->amsg.len = len;
		/* notice we over-allocate by LWS_PRE */
		vhd->amsg.payload = malloc(LWS_PRE + len);
		if (!vhd->amsg.payload) {
			lwsl_user("OOM: dropping\n");
			break;
		}

		memcpy((char *)vhd->amsg.payload + LWS_PRE, in, len);
		vhd->current++;

		/*
		 * let everybody know we want to write something on them
		 * as soon as they are ready
		 */
		lws_start_foreach_llp(struct per_session_data__minimal **,
				      ppss, vhd->pss_list) {
			lws_callback_on_writable((*ppss)->wsi);
		} lws_end_foreach_llp(ppss, pss_list);
		break;

	default:
		break;
	}

	return 0;
}

#define LWS_PLUGIN_PROTOCOL_MINIMAL \
	{ \
		"lws-minimal", \
		callback_minimal, \
		sizeof(struct per_session_data__minimal), \
		128, \
		0, NULL, 0 \
	}
