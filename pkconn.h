/******************************************************************************
pkconn.h - Connection objects

This file is Copyright 2011, 2012, The Beanstalks Project ehf.

This program is free software: you can redistribute it and/or modify it under
the terms of the  GNU  Affero General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,  but  WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for more
details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see: <http://www.gnu.org/licenses/>

Note: For alternate license terms, see the file COPYING.md.

******************************************************************************/

#define BLOCKING_FLUSH 1
#define NON_BLOCKING_FLUSH 0

/* These are the controlling parameters for our flow control - in order
 * to minimize buffer bloat, we want to keep the window relatively small
 * and assume that selecting a nearby front-end will keep this from
 * becoming too big a bottleneck on transfer speeds. */
#define CONN_WINDOW_SIZE_KB_MAXIMUM 256
#define CONN_WINDOW_SIZE_KB_MINIMUM  16
#define CONN_WINDOW_SIZE_STEPFACTOR  16 /* Lower: more aggressive/volatile */

typedef enum {
  CONN_TUNNEL_BLOCKED,
  CONN_TUNNEL_UNBLOCKED,
  CONN_DEST_BLOCKED,
  CONN_DEST_UNBLOCKED
} flow_op;

typedef enum {
  CONN_CLEAR_DATA,
#ifdef HAVE_OPENSSL
  CONN_SSL_DATA,
  CONN_SSL_HANDSHAKE,
#endif
} io_state_t;

#define CONN_IO_BUFFER_SIZE     PARSER_BYTES_MAX
#define CONN_STATUS_UNKNOWN     0x00000000
#define CONN_STATUS_END_READ    0x00000001 /* Don't want more data     */
#define CONN_STATUS_END_WRITE   0x00000002 /* Won't receive more data  */
#define CONN_STATUS_DST_BLOCKED 0x00000004 /* Destination blocked      */
#define CONN_STATUS_TNL_BLOCKED 0x00000008 /* Tunnel blocked           */
#define CONN_STATUS_BLOCKED    (0x00000008|0x00000004) /* Blocked      */
#define CONN_STATUS_CLS_READ    0x00000010 /* No more data available   */
#define CONN_STATUS_CLS_WRITE   0x00000020 /* No more writing possible */
#define CONN_STATUS_BROKEN     (0x00000040|0x10|0x20) /* ... broken.   */
#define CONN_STATUS_ALLOCATED   0x00000080
#define CONN_STATUS_BITS        0x000000FF
#define PKC_OUT(c)      ((c).out_buffer + (c).out_buffer_pos)
#define PKC_OUT_FREE(c) (CONN_IO_BUFFER_SIZE - (c).out_buffer_pos)
#define PKC_IN(c)       ((c).in_buffer + (c).in_buffer_pos)
#define PKC_IN_FREE(c)  (CONN_IO_BUFFER_SIZE - (c).in_buffer_pos)
struct pk_conn {
  int        status;
  int        sockfd;
  time_t     activity;
  size_t     read_bytes;
  size_t     read_kb;
  size_t     sent_kb;
  size_t     send_window_kb;
  int        in_buffer_pos;
  char       in_buffer[CONN_IO_BUFFER_SIZE];
  int        out_buffer_pos;
  char       out_buffer[CONN_IO_BUFFER_SIZE];
  ev_io      watch_r;
  ev_io      watch_w;
  io_state_t state_r;
  io_state_t state_w;
#ifdef HAVE_OPENSSL
  SSL*       ssl;
#endif
};

void    pkc_reset_conn(struct pk_conn*);
int     pkc_connect(struct pk_conn*, struct addrinfo*);
#ifdef HAVE_OPENSSL
int     pkc_start_ssl(struct pk_conn*);
#endif
int     pkc_wait(struct pk_conn*, int);
ssize_t pkc_read(struct pk_conn*);
ssize_t pkc_flush(struct pk_conn*, char*, ssize_t, int, char*);
ssize_t pkc_write(struct pk_conn*, char*, ssize_t);
