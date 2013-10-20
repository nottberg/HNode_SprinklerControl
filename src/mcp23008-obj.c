/*
 *  Copyright (c) cnottberg (2006)
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * ----------------------------------------------------------------------------
 *
 *  Description:
 *
 *  G-Object for communicating with a MCP23008 over i2c.
 */

#include <glib.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <linux/i2c-dev.h>

#include "mcp23008-obj.h"

typedef enum MCP230xxRegisterAddresses
{
    MCP23008_IODIR  = 0x00,
    MCP23008_GPIO   = 0x09,
    MCP23008_GPPU   = 0x06,
    MCP23008_OLAT   = 0x0A,
}MCP_REG_ADDR;

#define G_MCP23008_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), G_TYPE_MCP23008, GMCP23008Private))

G_DEFINE_TYPE (GMCP23008, g_mcp23008, G_TYPE_OBJECT);

typedef struct _GMCP23008Source GMCP23008Source;
struct _GMCP23008Source
{
    GSource     Source;
    GMCP23008     *Parent;
};

typedef struct _GMCP23008Private GMCP23008Private;
struct _GMCP23008Private
{
    guint8  pullup;
    guint8  direction;

    guint8  currentState;

    gboolean i2cActive;

    guint i2cAddress;
    guint i2cBusIndex;

    guint32 i2cdev;

    // G-Object variables
    gboolean        dispose_has_run;

};

/*
enum
{
	PROP_0
};
*/

/* GObject callbacks */
static void g_mcp23008_set_property (GObject 	 *object,
					    guint	  prop_id,
					    const GValue *value,
					    GParamSpec	 *pspec);
static void g_mcp23008_get_property (GObject	 *object,
					    guint	  prop_id,
					    GValue	 *value,
					    GParamSpec	 *pspec);

//extern void g_cclosure_user_marshal_VOID__UINT_UINT_UINT_UINT_POINTER (GClosure     *closure,
//                                                                       GValue       *return_value,
//                                                                       guint         n_param_values,
//                                                                       const GValue *param_values,
//                                                                       gpointer      invocation_hint,
//                                                                       gpointer      marshal_data);

static GObjectClass *parent_class = NULL;

static void
g_mcp23008_dispose (GObject *obj)
{
    GMCP23008        *self = (GMCP23008 *)obj;
    GMCP23008Private *priv;

	priv = G_MCP23008_GET_PRIVATE(self);

    if(priv->dispose_has_run)
    {
        /* If dispose did already run, return. */
        return;
    }

    /* Make sure dispose does not run twice. */
    priv->dispose_has_run = TRUE;

    /* 
    * In dispose, you are supposed to free all types referenced from this
    * object which might themselves hold a reference to self. Generally,
    * the most simple solution is to unref all members on which you own a 
    * reference.
    */

    /* Chain up to the parent class */
    G_OBJECT_CLASS (parent_class)->dispose (obj);
}

static void
g_mcp23008_finalize (GObject *obj)
{
    GMCP23008 *self = (GMCP23008 *)obj;

    /* Chain up to the parent class */
    G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
g_mcp23008_class_init (GMCP23008Class *class)
{
	GObjectClass *o_class;
	int error;
	
	o_class = G_OBJECT_CLASS (class);

    o_class->dispose  = g_mcp23008_dispose;
    o_class->finalize = g_mcp23008_finalize;

	/*
	o_class->set_property = g_mcp23008_browser_set_property;
	o_class->get_property = g_mcp23008_browser_get_property;
	*/
	
	/* create signals */
	class->cmd_complete_id = g_signal_new (
			"cmd_complete",
			G_OBJECT_CLASS_TYPE (o_class),
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (GMCP23008Class, cmd_complete),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE,
			1, G_TYPE_POINTER);

	class->async_rx_id = g_signal_new (
			"async_rx",
			G_OBJECT_CLASS_TYPE (o_class),
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (GMCP23008Class, async_rx),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE,
			1, G_TYPE_POINTER);

    parent_class = g_type_class_peek_parent (class);      
	g_type_class_add_private (o_class, sizeof (GMCP23008Private));
}



static void
g_mcp23008_init (GMCP23008 *sb)
{
	GMCP23008Private *priv;
    //GMCP23008ServerSource  *NodeSource;

	priv = G_MCP23008_GET_PRIVATE (sb);

    priv->i2cActive = FALSE;

    // Init i2c Parameters
    priv->i2cAddress  = 0x20;
    priv->i2cBusIndex = 0;

    // Initilize the ObjID value
    priv->dispose_has_run = FALSE;

}

/*
static void
g_mcp23008_server_set_property (GObject 	*object,
				guint		 prop_id,
				const GValue	*value,
				GParamSpec	*pspec)
{
	GMCP23008Server *sb;
	GMCP23008ServerPrivate *priv;

	sb = G_HNODE_SERVER (object);
	priv = G_HNODE_SERVER_GET_PRIVATE (object);

	switch (prop_id)
	{
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id,
					pspec);
			break;
	}
}

static void
g_mcp23008_server_get_property (GObject		*object,
				guint		 prop_id,
				GValue		*value,
				GParamSpec	*pspec)
{
	GMCP23008Server *sb;
	GMCP23008ServerPrivate *priv;

	sb = G_HNODE_SERVER (object);
	priv = G_HNODE_SERVER_GET_PRIVATE (object);
	
	switch (prop_id)
	{
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id,
					pspec);
			break;
	}
}
*/

GMCP23008*
g_mcp23008_new (void)
{
	return g_object_new (G_TYPE_MCP23008, NULL);
}

gboolean 
g_mcp23008_set_i2c_address(GMCP23008 *MCP23008, guint busnum, guint address)
{
	GMCP23008Private *priv;

	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    priv->i2cAddress = address;
    priv->i2cBusIndex = busnum;

    return FALSE;
}

guint
g_mcp23008_get_address( GMCP23008 *MCP23008 )
{
	GMCP23008Private *priv;

	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    return priv->i2cAddress;
}

guint
g_mcp23008_get_busnum( GMCP23008 *MCP23008 )
{
	GMCP23008Private *priv;

	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    return priv->i2cBusIndex;
}

gboolean
g_mcp23008_set_debug(GMCP23008 *MCP23008, guint debug)
{
    return FALSE;
}

guint
g_mcp23008_get_debug(GMCP23008 *MCP23008)
{
    return 0;
}

gboolean
g_mcp23008_start(GMCP23008 *MCP23008)
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
    gchar            devfn[256];
	//GMCP23008Event    hbevent;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    // Set default pin values -- all inputs with pull-ups disabled.
    // Current OLAT (output) value is polled, not set.
    priv->direction = 0xFF;
    priv->pullup = 0;

    // Build the device string
    sprintf( devfn, "/dev/i2c-%d", priv->i2cBusIndex );
  
    printf( "mcp23008 start: %s, %x\n", devfn, priv->i2cAddress );

    // Attempt to open the i2c device 
    if( ( priv->i2cdev = open( devfn, O_RDWR ) ) < 0 ) 
    {
        printf( "Failed to acquire bus access and/or talk to slave.\n" ); 
        perror("Failed to open the i2c bus");
        return TRUE;
    }

    // Tell the device which endpoint we want to talk to.
    if( ioctl( priv->i2cdev, I2C_SLAVE, priv->i2cAddress ) < 0 )
    {
        printf( "Failed to acquire bus access and/or talk to slave.\n" );
        /* ERROR HANDLING; you can check errno to see what went wrong */
        return TRUE;
    }

    priv->i2cActive = TRUE;

    // Init the IO direction and pullup settings
    i2c_smbus_write_byte_data( priv->i2cdev, MCP23008_IODIR, priv->direction );
    i2c_smbus_write_byte_data( priv->i2cdev, MCP23008_GPPU, priv->pullup );

    // Read and intial value from the device.
    priv->currentState = i2c_smbus_read_byte_data( priv->i2cdev, MCP23008_OLAT );

    printf( "Initial Value Read: %d\n", priv->currentState );

    return FALSE;
}

gboolean
g_mcp23008_stop(GMCP23008 *MCP23008)
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	//GMCP23008Event    hbevent;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    close( priv->i2cdev );

    priv->i2cActive = FALSE;

    return FALSE;
}

// Set single pin to either INPUT or OUTPUT mode
gboolean 
g_mcp23008_set_port_mode( GMCP23008 *MCP23008, guint mode )
{
    GMCP23008Class   *class;
    GMCP23008Private *priv;
	
    class = G_MCP23008_GET_CLASS (MCP23008);
    priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    priv->direction = 0xFF & ~(mode);

    printf( "set_port_mode: 0x%x\n", priv->direction );

    // Update the register
    i2c_smbus_write_byte_data( priv->i2cdev, MCP23008_IODIR, priv->direction );

    return FALSE;
}

// Set single pin to either INPUT or OUTPUT mode
guint 
g_mcp23008_get_port_mode( GMCP23008 *MCP23008 )
{
    GMCP23008Class   *class;
    GMCP23008Private *priv;
	
    class = G_MCP23008_GET_CLASS (MCP23008);
    priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    // Update the register
    priv->direction = i2c_smbus_read_byte_data( priv->i2cdev, MCP23008_IODIR );

    return priv->direction;
}

// Set single pin to either INPUT or OUTPUT mode
gboolean 
g_mcp23008_set_pin_mode(GMCP23008 *MCP23008, int pin, int mode )
{
    GMCP23008Class   *class;
    GMCP23008Private *priv;
	
    class = G_MCP23008_GET_CLASS (MCP23008);
    priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    if( mode == MCP23008_PIN_INPUT )
    {
        priv->direction |= (1 << pin);
    }
    else
    {
        priv->direction &= ~(1 << pin);
    }

    // Update the register
    i2c_smbus_write_byte_data( priv->i2cdev, MCP23008_IODIR, priv->direction );

    return priv->direction;
}

gboolean 
g_mcp23008_set_port_pullup(GMCP23008 *MCP23008, guint pullup )
{
    GMCP23008Class   *class;
    GMCP23008Private *priv;
	
    class = G_MCP23008_GET_CLASS (MCP23008);
    priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    priv->pullup = pullup;

    printf( "set_port_pullup: 0x%x\n", priv->pullup );

    i2c_smbus_write_byte_data( priv->i2cdev, MCP23008_GPPU, priv->pullup );

    return priv->pullup;
}

// Set single pin to either INPUT or OUTPUT mode
guint 
g_mcp23008_get_port_pullup( GMCP23008 *MCP23008 )
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    // Update the register
    priv->pullup = i2c_smbus_read_byte_data( priv->i2cdev, MCP23008_GPPU );

    return priv->pullup;
}

gboolean 
g_mcp23008_set_pin_pullup(GMCP23008 *MCP23008, int pin, int enable )
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    if( enable )
    {
        priv->pullup |= (1 << pin);
    }
    else
    {
        priv->pullup &= ~(1 << pin);
    }

    i2c_smbus_write_byte_data( priv->i2cdev, MCP23008_GPPU, priv->pullup );

    return priv->pullup;
}

guint 
g_mcp23008_get_port_state( GMCP23008 *MCP23008 )
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    priv->currentState = i2c_smbus_read_byte_data( priv->i2cdev, MCP23008_GPIO );

    return priv->currentState;
}

gboolean 
g_mcp23008_check_pin_state(GMCP23008 *MCP23008, int pin )
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    priv->currentState = i2c_smbus_read_byte_data( priv->i2cdev, MCP23008_GPIO );

    return (priv->currentState >> pin) & 1;
}

gboolean 
g_mcp23008_set_port_state(GMCP23008 *MCP23008, guint value )
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    int new;

    priv->currentState = value;

    i2c_smbus_write_byte_data( priv->i2cdev, MCP23008_OLAT, new );

    return new;
}

gboolean 
g_mcp23008_set_pin_state(GMCP23008 *MCP23008, guint8 pin, guint8 value )
{
    GMCP23008Class   *class;
    GMCP23008Private *priv;
	
    class = G_MCP23008_GET_CLASS (MCP23008);
    priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    int new;

    printf( "set_pin_state: 0x%x, 0x%x, 0x%x\n", priv->currentState, pin, value );

    if( value )
    {
        new = priv->currentState | (1 << pin);
    }
    else
    {
        new = priv->currentState & ~(1 << pin);
    }

    printf("set_pin_state new: 0x%x\n", new);

    // Only write if pin value has changed:
    if( new != priv->currentState )
    {
        priv->currentState = new;
        i2c_smbus_write_byte_data( priv->i2cdev, MCP23008_OLAT, new );
    }

    return new;
}

#if 0
gboolean 
g_mcp23008_i2ctest(GMCP23008 *MCP23008)
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    int file;
    char *filename = "/dev/i2c-0";
    char buf[10] = {0};
    float data;
    char channel;
    int i;
   
    //g_mcp23008_i2c_init(MCP23008, 0x20, 8, 0, 1 );

    // Set pins 0, 1, 2 as outputs
    g_mcp23008_config(MCP23008, 0, MCP23008_PM_OUTPUT );
    g_mcp23008_config(MCP23008, 1, MCP23008_PM_OUTPUT );
    g_mcp23008_config(MCP23008, 2, MCP23008_PM_OUTPUT );
    
    // Set pin 3 to input with the pullup resistor enabled
    g_mcp23008_pullup(MCP23008, 3, 1, 0 );

    // Read pin 3 and display the results
    printf( "%d: %x\n", 3, g_mcp23008_input(MCP23008, 3, 0 ) );
    
    // Python speed test on output 0 toggling at max speed
    while(1)
    {
        g_mcp23008_output(MCP23008, 0, 1 );
        g_mcp23008_output(MCP23008, 1, 0 );
        sleep(1);
        g_mcp23008_output(MCP23008, 0, 0 );
        g_mcp23008_output(MCP23008, 1, 1 );
        sleep(1);
    }
 
#if 0
    if( (file = open(filename, O_RDWR)) < 0) 
    {
        perror("Failed to open the i2c bus");
        exit(1);
    }

    int addr = 0x20;  // The I2C address of the ADC
    if( ioctl( file, I2C_SLAVE, addr ) < 0 )
    {
        printf( "Failed to acquire bus access and/or talk to slave.\n" );
        /* ERROR HANDLING; you can check errno to see what went wrong */
       exit( 1 );
    }

    for (i = 0; i<4; i++)
    {
        // Using I2C Read
        if( read( file, buf, 2 ) != 2 ) 
        {
            /* ERROR HANDLING: i2c transaction failed */
            printf("Failed to read from the i2c bus.\n");
            perror("Failed read"); 
            printf("\n\n");
        }
        else
        {
            data = (float)((buf[0] & 0b00001111)<<8)+buf[1];
            data = data/4096*5;
            channel = ((buf[0] & 0b00110000)>>4);
            printf("Channel %02d Data:  %04f\n",channel,data);
        }
    }
#endif
}

#endif

