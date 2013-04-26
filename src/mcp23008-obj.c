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

//static struct termios saved_io;
//char dev_str[] = "/dev/ttyS0";

//unsigned char startbuf[] = {0x02, 0x02, 0x02};
//unsigned char cmdbuf[] = {0x63, 0x40, 0x54, 0x45, 0x41};
//unsigned char cmdbuf2[] = {0x63, 0x40, 0x54, 0x47, 0x41};
//unsigned char cmdbuf3[] = {0x63, 0x40, 0x4C, 0x45, 0x41};
//unsigned char cmdbuf4[] = {0x63, 0x40, 0x4C, 0x47, 0x41};





#define G_MCP23008_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), G_TYPE_MCP23008, GMCP23008Private))

G_DEFINE_TYPE (GMCP23008, g_mcp23008, G_TYPE_OBJECT);

typedef struct _GMCP23008Source GMCP23008Source;
struct _GMCP23008Source
{
    GSource     Source;
    GMCP23008     *Parent;
};

// Map TW523 codes to regular X10 codes
typedef struct MCP23008CodeMap 
{
    guint8 CodeByte;
    guint8 RealByte;
}MCP23008_CODE_MAP;

static MCP23008_CODE_MAP gHouseMap[] =
{ 
    {0x46,'A'},
    {0x4E,'B'},
    {0x42,'C'},
    {0x4A,'D'},
    {0x41,'E'},
    {0x49,'F'},
    {0x45,'G'},
    {0x4D,'H'},
    {0x47,'I'},
    {0x4F,'J'},
    {0x43,'K'},
    {0x4B,'L'},
    {0x40,'M'},
    {0x48,'N'},
    {0x44,'O'},
    {0x4C,'P'},
    {0xFF, 0xFF},
};

static MCP23008_CODE_MAP gUnitMap[] =
{ 
    {0x4C, 1},
    {0x5C, 2},
    {0x44, 3},
    {0x54, 4},
    {0x42, 5},
    {0x52, 6},
    {0x4A, 7},
    {0x5A, 8},
    {0x4E, 9},
    {0x5E, 10},
    {0x46, 11},
    {0x56, 12},
    {0x40, 13},
    {0x50, 14},
    {0x48, 15},
    {0x58, 16},
    {0xFF, 0xFF},
};


#if 0
static MCP23008_CODE_MAP gFuncMap[] =
{ 
    {0x41, MCP23008_FUNC_ALL_UNITS_OFF},
    {0x43, MCP23008_FUNC_ALL_LIGHT_ON},
    {0x45, MCP23008_FUNC_ON},
    {0x47, MCP23008_FUNC_OFF},
    {0x49, MCP23008_FUNC_DIM},
    {0x4B, MCP23008_FUNC_BRIGHT},
    {0x4D, MCP23008_FUNC_ALL_LIGHTS_OFF},
    {0x4F, MCP23008_FUNC_EXTENDED_CODE},
    {0x51, MCP23008_FUNC_HAIL_REQUEST},
    {0x57, MCP23008_FUNC_PRESET_DIM_HIGH},
    {0x55, MCP23008_FUNC_PRESET_DIM_LOW},
    {0x59, MCP23008_FUNC_EXTENDED_DATA_ANALOG},
    {0x5B, MCP23008_FUNC_STATUS_ON},
    {0x5D, MCP23008_FUNC_STATUS_OFF},
    {0x5F, MCP23008_FUNC_STATUS_REQUEST},
    {0xFF, 0xFF},
};
#endif

typedef enum GMCP23008TxStates
{
    GILNK_TXS_IDLE      = 0,
    GILNK_TXS_PREAMBLE  = 1,
    GILNK_TXS_ACKWAIT   = 2,
    GILNK_TXS_CR        = 3,
    GILNK_TXS_CMDCODE   = 4,
    GILNK_TXS_HOUSE     = 5,
    GILNK_TXS_UNIT      = 6,
    GILNK_TXS_FUNC      = 7,
    GILNK_TXS_REPEAT    = 8,
    GILNK_TXS_CMDECHO   = 9
}G_MCP23008_TX_STATES;

typedef enum GMCP23008RxStates
{
    GILNK_RXS_IDLE       = 0,  // Waiting for receive activity

    GILNK_RXS_START1  = 1,  // Receiving a transmission confirmation
    GILNK_RXS_HOUSE1  = 2,
    GILNK_RXS_UNIT    = 3,
    GILNK_RXS_REPEAT1 = 4,
    GILNK_RXS_START2  = 5,
    GILNK_RXS_HOUSE2  = 6,
    GILNK_RXS_FUNC    = 7,
    GILNK_RXS_REPEAT2 = 8,

    GILNK_RXS_ACK     = 17,
}G_MCP23008_RX_STATES;

typedef struct _GMCP23008Private GMCP23008Private;
struct _GMCP23008Private
{
    guint8  num_gpios;
    guint8  pullups;
    guint8  outputvalue;
    guint8  direction;
    guint32 i2cdev;
    gchar  *devfn;
    guint32 addr;  // The I2C address of the ADC

    // Variables to deal with the serial link   
    struct termios saved_io;
	struct termios io;

    GMCP23008Source  *Source;
    GPollFD        Serial;

    guint32 TxState;
    guint32 RxState;

    gboolean         RxActive;
    gboolean         TxEcho;
    MCP23008_CMD_RECORD RxRecord;

    gboolean         TxActive;
    MCP23008_CMD_RECORD TxRecord;

    // G-Object variables
    gboolean        dispose_has_run;

};

static gboolean mcp23008_PrepareFunc(GSource *source, gint *timeout);
static gboolean mcp23008_CheckFunc(GSource *source);
static gboolean mcp23008_DispatchFunc(GSource *source, GSourceFunc callback, gpointer userdata);

static GSourceFuncs source_funcs = {
     mcp23008_PrepareFunc,
     mcp23008_CheckFunc,
     mcp23008_DispatchFunc,
    NULL,
    NULL,
    NULL
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

    // Get a source object to use when hooking into GLIB.
    priv->Source = (GMCP23008Source *) g_source_new(&source_funcs, sizeof(GMCP23008Source));
    priv->Source->Parent = sb;
        
    // Initilize the ObjID value
	priv->RxState = GILNK_RXS_IDLE;
	priv->TxState = GILNK_TXS_IDLE;

    priv->RxActive       = FALSE;
	priv->TxEcho         = FALSE;
    priv->RxRecord.u.raw = 0;

    priv->TxActive       = FALSE;
    priv->TxRecord.u.raw = 0;

    priv->dispose_has_run = FALSE;

    // Hook into the main loop
    g_main_context_ref( g_main_context_default() );
    g_source_attach((GSource *)priv->Source, g_main_context_default());

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

static guint8
g_mcp23008_map_code_to_real(GMCP23008 *MCP23008, MCP23008_CODE_MAP *Map, guint8 CodeByte)
{
    guint i;

    i = 0;
    while(Map[i].CodeByte != 0xFF)
    {
        if( Map[i].CodeByte == CodeByte )
            return Map[i].RealByte;
        i++;
    }
    
    // Didn't find a mapping
    return 0xFF;
}

static guint8
g_mcp23008_map_real_to_code(GMCP23008 *MCP23008, MCP23008_CODE_MAP *Map, guint8 RealByte)
{
    guint i;

    i = 0;
    while(Map[i].RealByte != 0xFF)
    {
        if( Map[i].RealByte == RealByte )
            return Map[i].CodeByte;
        i++;
    }
    
    // Didn't find a mapping
    return 0xFF;
}  

GMCP23008*
g_mcp23008_new (void)
{
	return g_object_new (G_TYPE_MCP23008, NULL);
}

gboolean
g_mcp23008_send_cmd(GMCP23008 *MCP23008, guint8 House, guint8 Unit, guint8 Function, guint8 Repeat)
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    if( priv->TxActive || priv->RxActive )
        return TRUE;

    //priv->TxRecord.u.b.House    = g_mcp23008_map_real_to_code( MCP23008, gHouseMap, House);
    //priv->TxRecord.u.b.Unit     = g_mcp23008_map_real_to_code( MCP23008, gUnitMap, Unit);
    //priv->TxRecord.u.b.Function = g_mcp23008_map_real_to_code( MCP23008, gFuncMap, Function);
    priv->TxRecord.u.b.Repeat   = (Repeat + 0x40);

    priv->TxActive = TRUE;
    priv->TxState  = GILNK_TXS_PREAMBLE;

    // Enable G_IO_OUT so that we get a tx-event
    priv->Serial.events  |= G_IO_OUT;

    g_source_remove_poll((GSource *)priv->Source, &priv->Serial);
    g_source_add_poll((GSource *)priv->Source, &priv->Serial);

    return FALSE;
}

gboolean
g_mcp23008_start(GMCP23008 *MCP23008, gchar *SerialDevStr)
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	//GMCP23008Event    hbevent;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

	priv->Serial.fd = open(SerialDevStr, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (priv->Serial.fd < 0) {
		g_error("Error opening iplc device %s\n", SerialDevStr);
		return TRUE;
	}

    printf("Serial File Descriptor: 0x%x\n", priv->Serial.fd);

	/* Save current serial port settings */
	tcgetattr(priv->Serial.fd, &priv->saved_io);

	/*
	 * Set up serial port for communication with PLC
	 *    9600 baud 8bit no parity
	 */
	priv->io.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
	priv->io.c_iflag = IGNPAR | IGNBRK;
	priv->io.c_oflag = 0;
	priv->io.c_lflag = 0;

	tcsetattr(priv->Serial.fd, TCSANOW, &priv->io);
 
    priv->Serial.events  = G_IO_IN | G_IO_ERR | G_IO_HUP; // G_IO_OUT
    priv->Serial.revents = 0;
       
    g_source_add_poll((GSource *)priv->Source, &priv->Serial);

     /* Check if the call suceeded */
//     if (version == NULL)
//     {
//         g_error ("Error getting version string: %s", avahi_strerror (avahi_client_errno (priv->AvahiClient)));
// 
//         goto fail;
//     }
         
    return FALSE;

    fail:
    /* Clean up */
	/* Restore the serial port settings */
	tcsetattr(priv->Serial.fd, TCSANOW, &priv->saved_io);

	/* Close the file descriptor */
	close(priv->Serial.fd);

    return TRUE;
}

static gboolean 
mcp23008_PrepareFunc(GSource *source, gint *timeout)
{
    GMCP23008Source  *Source = (GMCP23008Source *)source;
    GMCP23008        *MCP23008 = Source->Parent;
    GMCP23008Private *priv;

	priv = G_MCP23008_GET_PRIVATE( MCP23008 );

   *timeout = -1;
        
    return FALSE;
}

static gboolean 
mcp23008_CheckFunc(GSource *source)
{
    GMCP23008Source  *Source = (GMCP23008Source *)source;
    GMCP23008        *MCP23008 = Source->Parent;
    GMCP23008Private *priv;

	priv = G_MCP23008_GET_PRIVATE( MCP23008 );

    if( priv->Serial.revents > 0 )
    {
        return TRUE;
    }
              
    return FALSE;
}

static gboolean
mcp23008_process_tx_char(GMCP23008 *MCP23008)
{
    GMCP23008Class   *class;
	GMCP23008Private *priv;

    guint8 startchar = 0x02;
    guint8 cmdchar   = 0x63;

    gint status;

	class = G_MCP23008_GET_CLASS( MCP23008 );
	priv  = G_MCP23008_GET_PRIVATE( MCP23008 );

    switch( priv->TxState )
    {

        case GILNK_TXS_PREAMBLE:
            status = write(priv->Serial.fd, &startchar, 1);

            // Next state
            priv->TxState = GILNK_TXS_ACKWAIT; 

            // Wait for the receiver.
            return TRUE;

        case GILNK_TXS_CMDCODE:
            status = write(priv->Serial.fd, &cmdchar, 1);

            // Next state
            priv->TxState = GILNK_TXS_HOUSE; 
        break;

        case GILNK_TXS_HOUSE: 
            status = write(priv->Serial.fd, &(priv->TxRecord.u.b.House), 1);

            // Next state
            priv->TxState = GILNK_TXS_UNIT; 
        break;

        case GILNK_TXS_UNIT:  
            status = write(priv->Serial.fd, &(priv->TxRecord.u.b.Unit), 1);

            // Next state
            priv->TxState = GILNK_TXS_FUNC; 
        break;

        case GILNK_TXS_FUNC:  
            status = write(priv->Serial.fd, &(priv->TxRecord.u.b.Function), 1);

            // Next state
            priv->TxState = GILNK_TXS_REPEAT; 
        break;

        case GILNK_TXS_REPEAT:
            status = write(priv->Serial.fd, &(priv->TxRecord.u.b.Repeat), 1);        

            // Next state
            priv->TxState = GILNK_TXS_CMDECHO; 

            // Wait for the receiver
            return TRUE;

    }
    
    // Continue transmitting
    return FALSE;
}


static void
mcp23008_process_rx_char(GMCP23008 *MCP23008, gchar buf)
{
    GMCP23008Class   *class;
	GMCP23008Private *priv;

	class = G_MCP23008_GET_CLASS( MCP23008 );
	priv  = G_MCP23008_GET_PRIVATE( MCP23008 );

    // Check all of the tx handshakes
    if( priv->TxActive )
    {
        // Ignore CR                    
        if( (buf == 0xd) && ( priv->TxState == GILNK_TXS_CR) )
        {
            // Move to sending the command
            priv->TxState = GILNK_TXS_CMDCODE;
            
            // Enable G_IO_OUT so that we get a tx-event
            priv->Serial.events  |= G_IO_OUT;

            g_source_remove_poll((GSource *)priv->Source, &priv->Serial);
            g_source_add_poll((GSource *)priv->Source, &priv->Serial);
            
            return;
        }
        else if( (buf == 0x06) && (priv->TxState == GILNK_TXS_ACKWAIT) )
        {
            // Wait for the carriage return that should follow.
            priv->TxState = GILNK_TXS_CR;

            return;
        }
        else if( (buf == 0x15) && (priv->TxState == GILNK_TXS_ACKWAIT) )
        {
            // Try again to start a command send
            priv->TxState = GILNK_TXS_PREAMBLE;

            // Enable G_IO_OUT so that we get a tx-event
            priv->Serial.events  |= G_IO_OUT;

            g_source_remove_poll((GSource *)priv->Source, &priv->Serial);
            g_source_add_poll((GSource *)priv->Source, &priv->Serial);

            return;
        }

    }

    // Ignore CR for rx events                    
    if(buf == 0xd)
    {
        return;
    }

    switch( priv->RxState )
    {

        // Just waiting for something to happen.
        case GILNK_RXS_IDLE:
            if( buf == 0x58 )
            {
                // Start of a command Tx Ack.
                priv->TxEcho = TRUE;
                priv->RxState = GILNK_RXS_HOUSE1;
            }
            else if( buf == 0x78 )
            {
               // An asynchronous reception is starting.
               priv->RxActive = TRUE;
               priv->TxEcho   = FALSE;
               priv->RxState  = GILNK_RXS_HOUSE1;
            }
            else
            {
                return;
            }
        break;

        case GILNK_RXS_HOUSE1:
            // Verify and Save the House code
            priv->RxRecord.u.b.House = g_mcp23008_map_code_to_real(MCP23008, gHouseMap, buf);

            // Next should be the Unit code
            priv->RxState = GILNK_RXS_UNIT; 
        break;

        case GILNK_RXS_UNIT:
            // Verify and Save the House code
            priv->RxRecord.u.b.Unit = g_mcp23008_map_code_to_real(MCP23008, gUnitMap, buf);

            // Next should be the Unit code
            priv->RxState = GILNK_RXS_REPEAT1; 
        break;

        case GILNK_RXS_REPEAT1:
            priv->RxState = GILNK_RXS_START2;
        break;

        case GILNK_RXS_START2:
            priv->RxState = GILNK_RXS_HOUSE2;
        break;

        case GILNK_RXS_HOUSE2:
            priv->RxState = GILNK_RXS_FUNC;
        break;

        case GILNK_RXS_FUNC:
            // Verify and Save the Func code
            //priv->RxRecord.u.b.Function = g_mcp23008_map_code_to_real(MCP23008, gFuncMap, buf);

            priv->RxState = GILNK_RXS_REPEAT2;
        break;

        case GILNK_RXS_REPEAT2:
            // Verify and Save the sent count
            priv->RxRecord.u.b.Repeat = (buf - 0x30);

            // Reception is complete
            // Generate an event
            if( priv->TxEcho == TRUE )
            {
                priv->TxActive = FALSE;
                g_signal_emit(MCP23008, class->cmd_complete_id, 0, priv->RxRecord.u.raw);
            }
            else
            {
                priv->RxActive = FALSE;
                g_signal_emit(MCP23008, class->async_rx_id, 0, priv->RxRecord.u.raw);
            }

            // Return to idle for next reception
            priv->RxState = GILNK_RXS_IDLE;
        break;

        default:
        break;
    }
    
    return;
}


static gboolean 
mcp23008_DispatchFunc(GSource *source, GSourceFunc callback, gpointer userdata) 
{
    GMCP23008Source  *Source = (GMCP23008Source *)source;
    GMCP23008        *MCP23008 = Source->Parent;
    GMCP23008Private *priv;

    int status;
    char buf;

	priv = G_MCP23008_GET_PRIVATE( MCP23008 );

    if( priv->Serial.revents > 0)
    {
        if( priv->Serial.revents & (G_IO_ERR | G_IO_HUP) )
        {
            // The other end of the socket has gone down.
            // Close things out an clean up.  Also tell the 
            // upper layer that things failed.
            close( priv->Serial.fd );

            // Call the State Callback to indicate received data.
            //g_signal_emit(PktSrc, g_hnode_pktsrc_signals[PKTSRC_STATE_EVENT], 0, 0);                    

            return FALSE;        
        }

        if( priv->Serial.revents & G_IO_IN )
        {
            status = read(priv->Serial.fd, &buf, 1);
    
            if(status > 0 )
            {
                g_print("Serial Output 1: 0x%x\n", buf);

                // Run the new character through the state machine.
                mcp23008_process_rx_char(MCP23008, buf);

            }
        }
       
        // Handle transmit requests.
        if( priv->Serial.revents & G_IO_OUT )
        {
            // Send the next character
            if( mcp23008_process_tx_char(MCP23008) == TRUE )
            {
                // Turn off G_IO_OUT
                priv->Serial.events  &= ~G_IO_OUT;

                g_source_remove_poll((GSource *)priv->Source, &priv->Serial);
                g_source_add_poll((GSource *)priv->Source, &priv->Serial);
            }

        }

        priv->Serial.revents = 0;
    }
                                        
    return TRUE;
}

gboolean 
g_mcp23008_i2c_init(GMCP23008 *MCP23008, int address, int num_gpios, int busnum, int debug)
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    // Set default pin values -- all inputs with pull-ups disabled.
    // Current OLAT (output) value is polled, not set.
    priv->direction = 0xFF;
    priv->pullups = 0;
  
    // Attempt to open the i2c device 
    if( ( priv->i2cdev = open( priv->devfn, O_RDWR ) ) < 0 ) 
    {
        perror("Failed to open the i2c bus");
        exit(1);
    }

    // Tell the device which endpoint we want to talk to.
    if( ioctl( priv->i2cdev, I2C_SLAVE, priv->addr ) < 0 )
    {
        printf( "Failed to acquire bus access and/or talk to slave.\n" );
        /* ERROR HANDLING; you can check errno to see what went wrong */
       exit( 1 );
    }

    // Init the IO direction and pullup settings
    i2c_smbus_write_byte_data( priv->i2cdev, MCP23008_IODIR, priv->direction );
    i2c_smbus_write_byte_data( priv->i2cdev, MCP23008_GPPU, priv->pullups );

    // Read and intial value from the device.
    priv->outputvalue = i2c_smbus_read_byte_data( priv->i2cdev, MCP23008_OLAT );

    printf( "Initial Value Read: %d\n", priv->outputvalue );
}

// Set single pin to either INPUT or OUTPUT mode
gboolean 
g_mcp23008_config(GMCP23008 *MCP23008, int pin, int mode )
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    if( mode == MCP23008_PM_INPUT )
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
g_mcp23008_pullup(GMCP23008 *MCP23008, int pin, int enable, int check )
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    if( check )
    {
        if( ( priv->direction & (1 << pin) ) != 0 )
        {
            fprintf(stderr, "Pin %d not set to input", pin );
            exit(1);
        }
    }

    if( enable )
    {
        priv->pullups |= (1 << pin);
    }
    else
    {
        priv->pullups &= ~(1 << pin);
    }

    i2c_smbus_write_byte_data( priv->i2cdev, MCP23008_GPPU, priv->pullups );

    return priv->pullups;
}

gboolean 
g_mcp23008_input(GMCP23008 *MCP23008, int pin, int check )
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    int value;

    if( check )
    {
        if( ( priv->direction & (1 << pin) ) != 0 )
        {
            fprintf(stderr, "Pin %d not set to input", pin );
            exit(1);
        }
    }

    value = i2c_smbus_read_byte_data( priv->i2cdev, MCP23008_GPIO );

    return (value >> pin) & 1;
}

gboolean 
g_mcp23008_output(GMCP23008 *MCP23008, int pin, int value )
{
	GMCP23008Class   *class;
	GMCP23008Private *priv;
	
	class = G_MCP23008_GET_CLASS (MCP23008);
	priv  = G_MCP23008_GET_PRIVATE (MCP23008);

    int new;

    if( value )
    {
        new = priv->outputvalue | (1 << pin);
    }
    else
    {
        new = priv->outputvalue & ~(1 << pin);
    }

    // Only write if pin value has changed:
    if( new != priv->outputvalue )
    {
        priv->outputvalue = new;
        i2c_smbus_write_byte_data( priv->i2cdev, MCP23008_OLAT, new );
    }

    return new;
}

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
   
    g_mcp23008_i2c_init(MCP23008, 0x20, 8, 0, 1 );

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

