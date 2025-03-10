/*!
 * \file        sccp_event.h
 * \brief       SCCP Event Header
 * \author      Marcello Ceschia <marcelloceschia [at] users.sourceforge.net>
 * \note        This program is free software and may be modified and distributed under the terms of the GNU Public License.
 *              See the LICENSE file at the top of the source tree.
 * \since       2009-09-02
 *
 * $Date: 2015-10-12 09:51:15 +0000 (Mon, 12 Oct 2015) $
 * $Revision: 6218 $  
 */
#pragma once

//#include <config.h>
//#include "common.h"
//#include "chan_sccp.h"

#define NUMBER_OF_EVENT_TYPES 10

#ifdef DEBUG
#define sccp_event_retain(_x) 		({sccp_event_t const __attribute__((unused)) *tmp_##__LINE__##X = _x;ast_assert(tmp_##__LINE__##X != NULL);sccp_refcount_retain(_x, __FILE__, __LINE__, __PRETTY_FUNCTION__);})
#define sccp_event_release(_x) 		({sccp_event_t const __attribute__((unused)) *tmp_##__LINE__##X = _x;ast_assert(tmp_##__LINE__##X != NULL);sccp_refcount_release(_x, __FILE__, __LINE__, __PRETTY_FUNCTION__);})
#else 
#define sccp_event_retain(_x) 		({ast_assert(_x != NULL);sccp_refcount_retain(_x, __FILE__, __LINE__, __PRETTY_FUNCTION__);})
#define sccp_event_release(_x) 		({ast_assert(_x != NULL);sccp_refcount_release(_x, __FILE__, __LINE__, __PRETTY_FUNCTION__);})
#endif
/*!
 * \brief SCCP Event Structure
 */
typedef struct sccp_event sccp_event_t;
typedef void (*sccp_event_callback_t) (const sccp_event_t * event);

extern struct sccp_event_subscriptions *sccp_event_listeners;

/*!
 * \brief SCCP Event Structure
 */
struct sccp_event {
	/*! 
	 * \brief SCCP Event Data Union
	 */
	union sccp_event_data {
		struct {
			sccp_line_t *line;									/*!< SCCP Line (required) */
		} lineCreated;											/*!< Event Line Created Structure */
		struct {
			sccp_device_t *device;									/*!< SCCP Device (required) */
		} deviceRegistered;										/*!< Event Device Registered Structure */
		struct {
			sccp_linedevices_t *linedevice;								/*!< SCCP device line (required) */
		} deviceAttached;										/*!< Event Device Attached Structure */

		struct {
			sccp_device_t *device;									/*!< SCCP device (required) */
			sccp_linedevices_t *optional_linedevice;						/*!< SCCP linedevice (optional) */
			sccp_feature_type_t featureType;							/*!< what feature is changed (required) */
		} featureChanged;										/*!< Event feature changed Structure */

		struct {
			sccp_line_t *line;									/*!< SCCP line (required) */
			sccp_device_t *optional_device;								/*!< SCCP device (optional) */
			uint8_t state;										/*!< state (required) */
		} lineStatusChanged;										/*!< Event feature changed Structure */

	} event;												/*!< SCCP Event Data Union */

	sccp_event_type_t type;											/*!< Event Type */
};														/*!< SCCP Event Structure */

typedef struct sccp_event_subscriber sccp_event_subscriber_t;

/*!
 * \brief SCCP Event Subscriber Structure
 */
struct sccp_event_subscriber {
	sccp_event_callback_t callback_function;
	sccp_event_type_t eventType;
};

void sccp_event_unsubscribe(sccp_event_type_t eventType, sccp_event_callback_t cb);
void sccp_event_subscribe(sccp_event_type_t eventType, sccp_event_callback_t cb, boolean_t allowASyncExecution);

void sccp_event_module_start(void);
void sccp_event_module_stop(void);

void sccp_event_fire(const sccp_event_t * event);
// kate: indent-width 8; replace-tabs off; indent-mode cstyle; auto-insert-doxygen on; line-numbers on; tab-indents on; keep-extra-spaces off; auto-brackets off;
