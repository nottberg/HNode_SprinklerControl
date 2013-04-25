/**
 * ilink-obj.h
 *
 * Implements an interface to the TW523 X-10 serial interface module.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 * 
 * It is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with avahi; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 * 
 * (c) 2005, Curtis Nottberg
 *
 * Authors:
 *   Curtis Nottberg
 */

#include <glib.h>
#include <glib-object.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#ifndef __G_ILINK_OBJ_H__
#define __G_ILINK_OBJ_H__

G_BEGIN_DECLS

// Enumerate the Function codes that can be used.
typedef enum ILinkX10FunctionEnumeration
{
    ILINK_FUNC_ALL_UNITS_OFF,
    ILINK_FUNC_ALL_LIGHT_ON,
    ILINK_FUNC_ON,
    ILINK_FUNC_OFF,
    ILINK_FUNC_DIM,
    ILINK_FUNC_BRIGHT,
    ILINK_FUNC_ALL_LIGHTS_OFF,
    ILINK_FUNC_EXTENDED_CODE,
    ILINK_FUNC_HAIL_REQUEST,
    ILINK_FUNC_PRESET_DIM_HIGH,
    ILINK_FUNC_PRESET_DIM_LOW,
    ILINK_FUNC_EXTENDED_DATA_ANALOG,
    ILINK_FUNC_STATUS_ON,
    ILINK_FUNC_STATUS_OFF,
    ILINK_FUNC_STATUS_REQUEST
}ILINK_X10_FUNC_CODES;

// Use this structure to pass tx and rx events to the TW523
typedef struct ILinkX10CommandRecord 
{
    union
    {
        struct
        {
            guint8 House;
            guint8 Unit; 
            guint8 Function;
            guint8 Repeat;
        }b;
        
        guint32 raw;
    }u;
}ILINK_CMD_RECORD;

#define G_TYPE_ILINK			(g_ilink_get_type ())
#define G_ILINK(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_ILINK, GILink))
#define G_ILINK_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_ILINK, GILinkClass))

typedef struct _GILink		GILink;
typedef struct _GILinkClass	GILinkClass;
typedef struct _GILinkEvent	GILinkEvent;

struct _GILink
{
	GObject parent;
};

struct _GILinkClass
{
	GObjectClass parent_class;

	/* Signal handlers */
	void (* cmd_complete)   (GILink *sb, gpointer event);
	void (* async_rx)	    (GILink *sb, gpointer event);

    guint32 cmd_complete_id;
    guint32 async_rx_id;
};

struct _GILinkEvent
{
	guint32          ObjID;
    guint8          *PktPtr;
    guint32          PktLength;
};


GILink *g_ilink_new (void);

gboolean g_ilink_send_cmd(GILink *ILink, guint8 House, guint8 Unit, guint8 Function, guint8 Repeat);

G_END_DECLS

#endif
