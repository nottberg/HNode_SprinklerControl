/**
 * mcp23008-obj.h
 *
 * Implements an interface to a MCP23008 over i2c.
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

#ifndef __G_MCP23008_OBJ_H__
#define __G_MCP23008_OBJ_H__

G_BEGIN_DECLS

typedef enum MCP230xxPinMode
{
    MCP23008_PM_INPUT,
    MCP23008_PM_OUTPUT
}MCP_PIN_MODE;

typedef enum MCP230xxRegisterAddresses
{
    MCP23008_IODIR  = 0x00,
    MCP23008_GPIO   = 0x09,
    MCP23008_GPPU   = 0x06,
    MCP23008_OLAT   = 0x0A,
}MCP_REG_ADDR;

// Use this structure to pass tx and rx events to the TW523
typedef struct MCP23008X10CommandRecord 
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
}MCP23008_CMD_RECORD;

#define G_TYPE_MCP23008			(g_mcp23008_get_type ())
#define G_MCP23008(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_MCP23008, GILink))
#define G_MCP23008_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), G_TYPE_MCP23008, GMCP23008Class))

typedef struct _GMCP23008		GMCP23008;
typedef struct _GMCP23008Class	GMCP23008Class;
typedef struct _GMCP23008Event	GMCP23008Event;

struct _GMCP23008
{
	GObject parent;
};

struct _GMCP23008Class
{
	GObjectClass parent_class;

	/* Signal handlers */
	void (* cmd_complete)   (GMCP23008 *sb, gpointer event);
	void (* async_rx)	    (GMCP23008 *sb, gpointer event);

    guint32 cmd_complete_id;
    guint32 async_rx_id;
};

struct _GMCP23008Event
{
	guint32          ObjID;
    guint8          *PktPtr;
    guint32          PktLength;
};


GMCP23008 *g_mcp23008_new (void);

gboolean g_mcp23008_send_cmd(GMCP23008 *MCP23008, guint8 House, guint8 Unit, guint8 Function, guint8 Repeat);

gboolean g_mcp23008_i2c_init(GMCP23008 *MCP23008, int address, int num_gpios, int busnum, int debug);

// Set single pin to either INPUT or OUTPUT mode
gboolean g_mcp23008_config(GMCP23008 *MCP23008, int pin, int mode );

gboolean g_mcp23008_pullup(GMCP23008 *MCP23008, int pin, int enable, int check );

gboolean g_mcp23008_input(GMCP23008 *MCP23008, int pin, int check );

gboolean g_mcp23008_output(GMCP23008 *MCP23008, int pin, int value );

gboolean g_mcp23008_i2ctest(GMCP23008 *MCP23008);


G_END_DECLS

#endif // __G_I2C_MCP23008_OBJ_H__
