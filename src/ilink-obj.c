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
 *  G-Object for communicating with a TW523 X-10 Module.
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

#include "ilink-obj.h"

//static struct termios saved_io;
//char dev_str[] = "/dev/ttyS0";

//unsigned char startbuf[] = {0x02, 0x02, 0x02};
//unsigned char cmdbuf[] = {0x63, 0x40, 0x54, 0x45, 0x41};
//unsigned char cmdbuf2[] = {0x63, 0x40, 0x54, 0x47, 0x41};
//unsigned char cmdbuf3[] = {0x63, 0x40, 0x4C, 0x45, 0x41};
//unsigned char cmdbuf4[] = {0x63, 0x40, 0x4C, 0x47, 0x41};





#define G_ILINK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), G_TYPE_ILINK, GILinkPrivate))

G_DEFINE_TYPE (GILink, g_ilink, G_TYPE_OBJECT);

typedef struct _GILinkSource GILinkSource;
struct _GILinkSource
{
    GSource     Source;
    GILink     *Parent;
};

// Map TW523 codes to regular X10 codes
typedef struct ILinkCodeMap 
{
    guint8 CodeByte;
    guint8 RealByte;
}ILINK_CODE_MAP;

static ILINK_CODE_MAP gHouseMap[] =
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

static ILINK_CODE_MAP gUnitMap[] =
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



static ILINK_CODE_MAP gFuncMap[] =
{ 
    {0x41, ILINK_FUNC_ALL_UNITS_OFF},
    {0x43, ILINK_FUNC_ALL_LIGHT_ON},
    {0x45, ILINK_FUNC_ON},
    {0x47, ILINK_FUNC_OFF},
    {0x49, ILINK_FUNC_DIM},
    {0x4B, ILINK_FUNC_BRIGHT},
    {0x4D, ILINK_FUNC_ALL_LIGHTS_OFF},
    {0x4F, ILINK_FUNC_EXTENDED_CODE},
    {0x51, ILINK_FUNC_HAIL_REQUEST},
    {0x57, ILINK_FUNC_PRESET_DIM_HIGH},
    {0x55, ILINK_FUNC_PRESET_DIM_LOW},
    {0x59, ILINK_FUNC_EXTENDED_DATA_ANALOG},
    {0x5B, ILINK_FUNC_STATUS_ON},
    {0x5D, ILINK_FUNC_STATUS_OFF},
    {0x5F, ILINK_FUNC_STATUS_REQUEST},
    {0xFF, 0xFF},
};

typedef enum GILinkTxStates
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
}G_ILINK_TX_STATES;

typedef enum GILinkRxStates
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
}G_ILINK_RX_STATES;

typedef struct _GILinkPrivate GILinkPrivate;
struct _GILinkPrivate
{

    // Variables to deal with the serial link   
    struct termios saved_io;
	struct termios io;

    GILinkSource  *Source;
    GPollFD        Serial;

    guint32 TxState;
    guint32 RxState;

    gboolean         RxActive;
    gboolean         TxEcho;
    ILINK_CMD_RECORD RxRecord;

    gboolean         TxActive;
    ILINK_CMD_RECORD TxRecord;

    // G-Object variables
    gboolean        dispose_has_run;

};

static gboolean ilink_PrepareFunc(GSource *source, gint *timeout);
static gboolean ilink_CheckFunc(GSource *source);
static gboolean ilink_DispatchFunc(GSource *source, GSourceFunc callback, gpointer userdata);

static GSourceFuncs source_funcs = {
     ilink_PrepareFunc,
     ilink_CheckFunc,
     ilink_DispatchFunc,
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
static void g_ilink_set_property (GObject 	 *object,
					    guint	  prop_id,
					    const GValue *value,
					    GParamSpec	 *pspec);
static void g_ilink_get_property (GObject	 *object,
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
g_ilink_dispose (GObject *obj)
{
    GILink        *self = (GILink *)obj;
    GILinkPrivate *priv;

	priv = G_ILINK_GET_PRIVATE(self);

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
g_ilink_finalize (GObject *obj)
{
    GILink *self = (GILink *)obj;

    /* Chain up to the parent class */
    G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
g_ilink_class_init (GILinkClass *class)
{
	GObjectClass *o_class;
	int error;
	
	o_class = G_OBJECT_CLASS (class);

    o_class->dispose  = g_ilink_dispose;
    o_class->finalize = g_ilink_finalize;

	/*
	o_class->set_property = g_ilink_browser_set_property;
	o_class->get_property = g_ilink_browser_get_property;
	*/
	
	/* create signals */
	class->cmd_complete_id = g_signal_new (
			"cmd_complete",
			G_OBJECT_CLASS_TYPE (o_class),
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (GILinkClass, cmd_complete),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE,
			1, G_TYPE_POINTER);

	class->async_rx_id = g_signal_new (
			"async_rx",
			G_OBJECT_CLASS_TYPE (o_class),
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (GILinkClass, async_rx),
			NULL, NULL,
			g_cclosure_marshal_VOID__POINTER,
			G_TYPE_NONE,
			1, G_TYPE_POINTER);

    parent_class = g_type_class_peek_parent (class);      
	g_type_class_add_private (o_class, sizeof (GILinkPrivate));
}



static void
g_ilink_init (GILink *sb)
{
	GILinkPrivate *priv;
    //GILinkServerSource  *NodeSource;

	priv = G_ILINK_GET_PRIVATE (sb);

    // Get a source object to use when hooking into GLIB.
    priv->Source = (GILinkSource *) g_source_new(&source_funcs, sizeof(GILinkSource));
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
g_ilink_server_set_property (GObject 	*object,
				guint		 prop_id,
				const GValue	*value,
				GParamSpec	*pspec)
{
	GILinkServer *sb;
	GILinkServerPrivate *priv;

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
g_ilink_server_get_property (GObject		*object,
				guint		 prop_id,
				GValue		*value,
				GParamSpec	*pspec)
{
	GILinkServer *sb;
	GILinkServerPrivate *priv;

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
g_ilink_map_code_to_real(GILink *ILink, ILINK_CODE_MAP *Map, guint8 CodeByte)
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
g_ilink_map_real_to_code(GILink *ILink, ILINK_CODE_MAP *Map, guint8 RealByte)
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

GILink*
g_ilink_new (void)
{
	return g_object_new (G_TYPE_ILINK, NULL);
}

gboolean
g_ilink_send_cmd(GILink *ILink, guint8 House, guint8 Unit, guint8 Function, guint8 Repeat)
{
	GILinkClass   *class;
	GILinkPrivate *priv;
	
	class = G_ILINK_GET_CLASS (ILink);
	priv  = G_ILINK_GET_PRIVATE (ILink);

    if( priv->TxActive || priv->RxActive )
        return TRUE;

    priv->TxRecord.u.b.House    = g_ilink_map_real_to_code( ILink, gHouseMap, House);
    priv->TxRecord.u.b.Unit     = g_ilink_map_real_to_code( ILink, gUnitMap, Unit);
    priv->TxRecord.u.b.Function = g_ilink_map_real_to_code( ILink, gFuncMap, Function);
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
g_ilink_start(GILink *ILink, gchar *SerialDevStr)
{
	GILinkClass   *class;
	GILinkPrivate *priv;
	//GILinkEvent    hbevent;
	
	class = G_ILINK_GET_CLASS (ILink);
	priv  = G_ILINK_GET_PRIVATE (ILink);

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
ilink_PrepareFunc(GSource *source, gint *timeout)
{
    GILinkSource  *Source = (GILinkSource *)source;
    GILink        *ILink = Source->Parent;
    GILinkPrivate *priv;

	priv = G_ILINK_GET_PRIVATE( ILink );

   *timeout = -1;
        
    return FALSE;
}

static gboolean 
ilink_CheckFunc(GSource *source)
{
    GILinkSource  *Source = (GILinkSource *)source;
    GILink        *ILink = Source->Parent;
    GILinkPrivate *priv;

	priv = G_ILINK_GET_PRIVATE( ILink );

    if( priv->Serial.revents > 0 )
    {
        return TRUE;
    }
              
    return FALSE;
}

static gboolean
ilink_process_tx_char(GILink *ILink)
{
    GILinkClass   *class;
	GILinkPrivate *priv;

    guint8 startchar = 0x02;
    guint8 cmdchar   = 0x63;

    gint status;

	class = G_ILINK_GET_CLASS( ILink );
	priv  = G_ILINK_GET_PRIVATE( ILink );

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
ilink_process_rx_char(GILink *ILink, gchar buf)
{
    GILinkClass   *class;
	GILinkPrivate *priv;

	class = G_ILINK_GET_CLASS( ILink );
	priv  = G_ILINK_GET_PRIVATE( ILink );

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
            priv->RxRecord.u.b.House = g_ilink_map_code_to_real(ILink, gHouseMap, buf);

            // Next should be the Unit code
            priv->RxState = GILNK_RXS_UNIT; 
        break;

        case GILNK_RXS_UNIT:
            // Verify and Save the House code
            priv->RxRecord.u.b.Unit = g_ilink_map_code_to_real(ILink, gUnitMap, buf);

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
            priv->RxRecord.u.b.Function = g_ilink_map_code_to_real(ILink, gFuncMap, buf);

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
                g_signal_emit(ILink, class->cmd_complete_id, 0, priv->RxRecord.u.raw);
            }
            else
            {
                priv->RxActive = FALSE;
                g_signal_emit(ILink, class->async_rx_id, 0, priv->RxRecord.u.raw);
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
ilink_DispatchFunc(GSource *source, GSourceFunc callback, gpointer userdata) 
{
    GILinkSource  *Source = (GILinkSource *)source;
    GILink        *ILink = Source->Parent;
    GILinkPrivate *priv;

    int status;
    char buf;

	priv = G_ILINK_GET_PRIVATE( ILink );

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
                ilink_process_rx_char(ILink, buf);

            }
        }
       
        // Handle transmit requests.
        if( priv->Serial.revents & G_IO_OUT )
        {
            // Send the next character
            if( ilink_process_tx_char(ILink) == TRUE )
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

