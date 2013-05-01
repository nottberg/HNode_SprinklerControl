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
    MCP23008_PIN_INPUT,
    MCP23008_PIN_OUTPUT
}MCP_PIN_MODE;

#define MCP23008_PM_PIN0_IN   (MCP23008_PIN_INPUT << 0)
#define MCP23008_PM_PIN0_OUT  (MCP23008_PIN_OUTPUT << 0)
#define MCP23008_PM_PIN1_IN   (MCP23008_PIN_INPUT << 1)
#define MCP23008_PM_PIN1_OUT  (MCP23008_PIN_OUTPUT << 1)
#define MCP23008_PM_PIN2_IN   (MCP23008_PIN_INPUT << 2)
#define MCP23008_PM_PIN2_OUT  (MCP23008_PIN_OUTPUT << 2)
#define MCP23008_PM_PIN3_IN   (MCP23008_PIN_INPUT << 3)
#define MCP23008_PM_PIN3_OUT  (MCP23008_PIN_OUTPUT << 3)
#define MCP23008_PM_PIN4_IN   (MCP23008_PIN_INPUT << 4)
#define MCP23008_PM_PIN4_OUT  (MCP23008_PIN_OUTPUT << 4)
#define MCP23008_PM_PIN5_IN   (MCP23008_PIN_INPUT << 5)
#define MCP23008_PM_PIN5_OUT  (MCP23008_PIN_OUTPUT << 5)
#define MCP23008_PM_PIN6_IN   (MCP23008_PIN_INPUT << 6)
#define MCP23008_PM_PIN6_OUT  (MCP23008_PIN_OUTPUT << 6)
#define MCP23008_PM_PIN7_IN   (MCP23008_PIN_INPUT << 7)
#define MCP23008_PM_PIN7_OUT  (MCP23008_PIN_OUTPUT << 7)

typedef enum MCP230xxPinPullup
{
    MCP23008_PIN_PU_ON,
    MCP23008_PIN_PU_OFF
}MCP_PIN_PULLUP;

#define MCP23008_PU_PIN0_ON   (MCP23008_PIN_PU_ON << 0)
#define MCP23008_PU_PIN0_OFF  (MCP23008_PIN_PU_OFF << 0)
#define MCP23008_PU_PIN1_ON   (MCP23008_PIN_PU_ON << 1)
#define MCP23008_PU_PIN1_OFF  (MCP23008_PIN_PU_OFF << 1)
#define MCP23008_PU_PIN2_ON   (MCP23008_PIN_PU_ON << 2)
#define MCP23008_PU_PIN2_OFF  (MCP23008_PIN_PU_OFF << 2)
#define MCP23008_PU_PIN3_ON   (MCP23008_PIN_PU_ON << 3)
#define MCP23008_PU_PIN3_OFF  (MCP23008_PIN_PU_OFF << 3)
#define MCP23008_PU_PIN4_ON   (MCP23008_PIN_PU_ON << 4)
#define MCP23008_PU_PIN4_OFF  (MCP23008_PIN_PU_OFF << 4)
#define MCP23008_PU_PIN5_ON   (MCP23008_PIN_PU_ON << 5)
#define MCP23008_PU_PIN5_OFF  (MCP23008_PIN_PU_OFF << 5)
#define MCP23008_PU_PIN6_ON   (MCP23008_PIN_PU_ON << 6)
#define MCP23008_PU_PIN6_OFF  (MCP23008_PIN_PU_OFF << 6)
#define MCP23008_PU_PIN7_ON   (MCP23008_PIN_PU_ON << 7)
#define MCP23008_PU_PIN7_OFF  (MCP23008_PIN_PU_OFF << 7)

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

gboolean g_mcp23008_set_i2c_address(GMCP23008 *MCP23008, guint busnum, guint address);
guint g_mcp23008_get_address( GMCP23008 *MCP23008 );
guint g_mcp23008_get_busnum( GMCP23008 *MCP23008 );

gboolean g_mcp23008_set_debug(GMCP23008 *MCP23008, guint debug);
guint g_mcp23008_get_debug(GMCP23008 *MCP23008);

gboolean g_mcp23008_start(GMCP23008 *MCP23008);
gboolean g_mcp23008_stop(GMCP23008 *MCP23008);

guint g_mcp23008_get_port_mode( GMCP23008 *MCP23008 );
gboolean g_mcp23008_set_port_mode(GMCP23008 *MCP23008, guint mode );
gboolean g_mcp23008_set_pin_mode(GMCP23008 *MCP23008, int pin, int mode );

guint g_mcp23008_get_port_pullup( GMCP23008 *MCP23008 );
gboolean g_mcp23008_set_port_pullup(GMCP23008 *MCP23008, guint pullup );
gboolean g_mcp23008_set_pin_pullup(GMCP23008 *MCP23008, int pin, int enable );

guint g_mcp23008_get_port_state( GMCP23008 *MCP23008 );
gboolean g_mcp23008_check_pin_state(GMCP23008 *MCP23008, int pin );
gboolean g_mcp23008_set_port_state(GMCP23008 *MCP23008, guint value );
gboolean g_mcp23008_set_pin_state(GMCP23008 *MCP23008, guint8 pin, guint8 value );

G_END_DECLS

#endif // __G_I2C_MCP23008_OBJ_H__
