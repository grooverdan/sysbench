/* Copyright (C) 2004 MySQL AB
   Copyright (C) 2004-2011 Alexey Kopytov <akopytov@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef SB_LOGGER_H
#define SB_LOGGER_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "sb_global.h"
#include "sb_options.h"
#include "sb_timer.h"

/* Text message flags (used in the 'flags' field of log_text_msg_t) */

#define LOG_MSG_TEXT_ALLOW_DUPLICATES 1

/* Macros to log per-request execution statistics */

#define LOG_EVENT_START(msg, thread_id) \
  do \
  { \
    ((log_msg_oper_t *)(msg).data)->thread_id = thread_id; \
    ((log_msg_oper_t *)(msg).data)->action = LOG_MSG_OPER_START; \
    log_msg(&(msg)); \
  } while (0);

#define LOG_EVENT_STOP(msg, thread_id) \
  do \
  { \
    ((log_msg_oper_t *)(msg).data)->thread_id = thread_id; \
    ((log_msg_oper_t *)(msg).data)->action = LOG_MSG_OPER_STOP; \
    log_msg(&(msg)); \
  } while (0);

/* Message types definition */

typedef enum {
  LOG_MSG_TYPE_MIN,           /* used for internal purposes */
  LOG_MSG_TYPE_TEXT,          /* arbitrary text messages */
  LOG_MSG_TYPE_STRUCTURE,     /* structured message for structured output */
  LOG_MSG_TYPE_OPER,          /* operation start/stop messages */
  LOG_MSG_TYPE_MAX            /* used for internal purposes */
} log_msg_type_t;

/* Message priorities definition */

typedef enum {
  LOG_FATAL,     /* system is unusable */
  LOG_ALERT,     /* user actions must be taken */
  LOG_WARNING,   /* warnings */
  LOG_NOTICE,    /* normal but significant messages */
  LOG_INFO,      /* informational messages */   
  LOG_DEBUG,     /* debug-level messages */
  LOG_MAX        /* used for internal purposes */
} log_msg_priority_t;

/* Text message definition */

typedef struct {
  log_msg_priority_t priority;
  char               *text;
  unsigned int       flags;
} log_msg_text_t;

/* Structed message definition */

typedef struct {
  const char         *tag;
  const char         *value;
  log_msg_priority_t priority;
} log_msg_struct_t;

/* Operation start/stop message definition */

typedef enum {
  LOG_MSG_OPER_START,
  LOG_MSG_OPER_STOP
} log_msg_oper_action_t;

typedef struct {
  log_msg_oper_action_t action;
  int                   thread_id;
} log_msg_oper_t;

/* General log message definition */

typedef struct {
  log_msg_type_t type;
  void           *data;
} log_msg_t;

/* Log handler operations definition */

typedef int log_op_register(void);          /* register handler options */
typedef int log_op_init(void);              /* initialize log handler */
typedef int log_op_process(log_msg_t *msg); /* process message */
typedef void log_op_begin_section(const char *section); /* begin section */
typedef void log_op_end_section(const char *section);   /* end section */
typedef int log_op_done(void);              /* uninitialize log handler */

/* Log handler operations structure */

typedef struct {
  log_op_init          *init;
  log_op_process       *process;
  log_op_begin_section *begin_section;
  log_op_end_section   *end_section;
  log_op_done          *done;
} log_handler_ops_t;

/* Log handler definition */

typedef struct {
  log_handler_ops_t ops;          /* handler operations */
  sb_arg_t             *args;     /* handler arguments */
  sb_list_item_t       listitem;  /* can be linked in a list */
} log_handler_t;

/* per-thread timers for response time stats */
extern sb_timer_t *timers;

/* Register logger */

int log_register(void);

/* Display command line options for all register log handlers */

void log_print_help(void);

/* Initialize logger */

int log_init(void);

/* Add handler for a specified type of messages */

int log_add_handler(log_msg_type_t type, log_handler_t *handler);

/* Remove handler for a specified type of messages */

int log_delete_handler(log_msg_type_t type, log_handler_t *handler);

/* Main function to dispatch log messages */

void log_msg(log_msg_t *);

/* printf-like wrapper to log text messages */

void log_text(log_msg_priority_t priority, const char *fmt, ...)
  SB_ATTRIBUTE_FORMAT(printf, 2, 3);

/* for outputing structured messages. Returns the contents of
 * snprint(buffer, MAX_STRUCTURE_LENGTH, fmt, ...) in buffer
 * as this will be used in the log_text */
#define MAX_STRUCTURE_LENGTH  40

void log_structure(log_msg_priority_t priority, const char *tag,
  char buffer[MAX_STRUCTURE_LENGTH], const char *fmt, ...)
  SB_ATTRIBUTE_FORMAT(printf, 4, 5);

/*
  variant of log_text() which prepends log lines with a elapsed time of the
  specified timer.
*/

void log_timestamp(log_msg_priority_t priority, const sb_timer_t *timer,
                   const char *fmt, ...)
  SB_ATTRIBUTE_FORMAT(printf, 3, 4);

/* printf-like wrapper to log system error messages */

void log_errno(log_msg_priority_t priority, const char *fmt, ...)
  SB_ATTRIBUTE_FORMAT(printf, 2, 3);

/* Begin/end section for structured output */

void log_begin_section(const char *section);

void log_end_section(const char *section);

/* Output the configuration of the test into output as structured */

void log_config(sb_arg_t args[]);

/* Uninitialize logger */

void log_done(void);

/*
  Print global stats either from the last checkpoint (if used) or
  from the test start.
*/

int print_global_stats(void);

#endif /* SB_LOGGER_H */
