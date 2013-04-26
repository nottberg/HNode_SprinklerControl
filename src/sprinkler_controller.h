/**
 * sprinklerController.h
 *
 * Implements an interface to a SPRINKLER_CTRL over i2c.
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
 * (c) 2013, Curtis Nottberg
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

#ifndef __G_SPRINKLER_CTRL_OBJ_H__
#define __G_SPRINKLER_CTRL_OBJ_H__

G_BEGIN_DECLS

typedef enum MCP230xxPinMode
{
    SPRINKLER_CTRL_PM_INPUT,
    SPRINKLER_CTRL_PM_OUTPUT
}MCP_PIN_MODE;

typedef enum MCP230xxRegisterAddresses
{
    SPRINKLER_CTRL_IODIR  = 0x00,
    SPRINKLER_CTRL_GPIO   = 0x09,
    SPRINKLER_CTRL_GPPU   = 0x06,
    SPRINKLER_CTRL_OLAT   = 0x0A,
}MCP_REG_ADDR;

// Use this structure to pass tx and rx events to the TW523
typedef struct SPRINKLER_CTRLX10CommandRecord 
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
}SPRINKLER_CTRL_CMD_RECORD;

#define G_TYPE_SPRINKLER_CTRL			(g_sprinkler_controller_get_type ())
#define G_SPRINKLER_CTRL(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_SPRINKLER_CTRL, GILink))
#define G_SPRINKLER_CTRL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_SPRINKLER_CTRL, GSPRINKLER_CTRLClass))

typedef struct _GSPRINKLER_CTRL		GSPRINKLER_CTRL;
typedef struct _GSPRINKLER_CTRLClass	GSPRINKLER_CTRLClass;
typedef struct _GSPRINKLER_CTRLEvent	GSPRINKLER_CTRLEvent;

struct _GSPRINKLER_CTRL
{
	GObject parent;
};

struct _GSPRINKLER_CTRLClass
{
	GObjectClass parent_class;

	/* Signal handlers */
	void (* cmd_complete)   (GSPRINKLER_CTRL *sb, gpointer event);
	void (* async_rx)	    (GSPRINKLER_CTRL *sb, gpointer event);

    guint32 cmd_complete_id;
    guint32 async_rx_id;
};

struct _GSPRINKLER_CTRLEvent
{
	guint32          ObjID;
    guint8          *PktPtr;
    guint32          PktLength;
};


GSPRINKLER_CTRL *g_sprinkler_controller_new (void);

gboolean g_sprinkler_controller_send_cmd(GSPRINKLER_CTRL *SPRINKLER_CTRL, guint8 House, guint8 Unit, guint8 Function, guint8 Repeat);

G_END_DECLS

#endif // __G_I2C_MCP23008_OBJ_H__
