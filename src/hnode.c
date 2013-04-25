/* $Id: glib-integration.c 937 2005-11-08 21:56:28Z lennart $ */
 

 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h> 
#include <glib.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>

#include <avahi-common/alternative.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>

#include <avahi-glib/glib-watch.h>
#include <avahi-glib/glib-malloc.h>
 
#include <hnode-1.0/hnode-pktsrc.h>
#include <hnode-1.0/hnode-nodeobj.h>

#include "hnode-switch-interface.h"
#include "ilink-obj.h"

guint8 gUID[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0xfe, 0xff};

static gint wait_time = 0;
static gint debug_period = 10000;
static gint event_period = 10000;
static gint up_period = 4000;
static gint down_period = 10000;
static gboolean debug = FALSE;
static gboolean event = FALSE;
static gboolean updown = FALSE;

static GOptionEntry entries[] = 
{
  { "device", 'd', 0, G_OPTION_ARG_INT, &wait_time, "The device path for the serial port.", "N" },
  { "x10cmd", 0, 0, G_OPTION_ARG_INT, &debug_period, "Send an x10 command and exit. Cmd String: <> <> <>", "N" },
  { "print", 0, 0, G_OPTION_ARG_INT, &event_period, "Turn on printing of rx and tx to standard out", "N" },
  { NULL }
};

typedef struct x10NodeContext
{
    GILink    *ILink;
    GHNode    *HNode;

    GHNodePktSrc    *X10Source;
    GHNodePktSrc    *SwitchSource;

    guint16  X10Port;
    guint16  SwitchPort;

}CONTEXT;

static struct termios saved_io;

/*
char dev_str[] = "/dev/ttyS0";

unsigned char startbuf[] = {0x02, 0x02, 0x02};
unsigned char cmdbuf[] = {0x63, 0x40, 0x54, 0x45, 0x41};
unsigned char cmdbuf2[] = {0x63, 0x40, 0x54, 0x47, 0x41};
unsigned char cmdbuf3[] = {0x63, 0x40, 0x4C, 0x45, 0x41};
unsigned char cmdbuf4[] = {0x63, 0x40, 0x4C, 0x47, 0x41};

GIOChannel *serio;

gboolean sendcmd = FALSE;
gboolean oncmd = TRUE;

static gboolean
hnode_updown_timeout (void *userdata)
{
//    GHNode    *HNode = userdata;
    GError *error = NULL;
    gint written;

    g_message ("hnode_updown_timeout!");

    sendcmd = TRUE;
     
    // Write the preamble character
    g_io_channel_write_chars(serio, startbuf, 1, &written, &error);
    g_io_channel_flush(serio, NULL);

    return TRUE; // Don't re-schedule timeout event 
}

gboolean
serio_callback(GIOChannel *ioch, GIOCondition cond, gpointer data)
{
  int bytes_read;
  unsigned char buf;
  gint written;
  GError *error = NULL;
 
  do {
    g_io_channel_read_chars(ioch, &buf, 1, &bytes_read, NULL);
    if (bytes_read)
    {
      //process_char(chr);
      printf("serial: %02x\n", buf);

      if( sendcmd == TRUE )
      {
        if(buf == 0xd)
        {
            g_io_channel_write_chars(serio, (oncmd == TRUE ? cmdbuf : cmdbuf2), sizeof(cmdbuf), &written, &error);
            g_io_channel_flush(serio, NULL);
            sendcmd = FALSE;
            oncmd = (oncmd == TRUE ? FALSE : TRUE); 
        }
        if(buf == 0x15)
            sendcmd = FALSE;
      }
    }                                           
  } while (bytes_read);

  return TRUE;
}
*/

void 
hnode_x10_rx(GHNodePktSrc *sb, GHNodePacket *Packet, gpointer data)
{
    CONTEXT  *Context = (CONTEXT *)data;

#if 0
    guint DataLength;
    guint PktType;
    guint ReqTag;

    GHNode        *HNode = (GHNode *) data;
	GHNodeClass   *class;
	GHNodePrivate *priv;

    GHNodePacket *TxPacket;

	class = G_HNODE_GET_CLASS (HNode);
	priv = G_HNODE_GET_PRIVATE (HNode);

    DataLength = g_hnode_packet_get_data_length(Packet);
    PktType    = g_hnode_packet_get_uint(Packet);
    ReqTag     = g_hnode_packet_get_uint(Packet);

	switch( PktType )
    {
        // Request for general info about this hnode.
        case HNPKT_PINFO_REQ:
            printf("Provider Info Request -- Tag: 0x%x\n", ReqTag);

            TxPacket = g_hnode_packet_new();						
    
            // Set the TX address
            g_hnode_packet_clone_address(TxPacket, Packet);

	        // Check to see if this is the packet we are interested in.
            g_hnode_packet_set_uint(TxPacket, HNPKT_PINFO_RPY);
            g_hnode_packet_set_uint(TxPacket, ReqTag);

    		g_hnode_packet_set_short(TxPacket, priv->MicroVersion); // Micro Version
    		g_hnode_packet_set_char(TxPacket, priv->MinorVersion);  // Minor Version
    		g_hnode_packet_set_char(TxPacket, priv->MajorVersion);  // Major Version

    		// Get the unique ID for the node.
            g_hnode_packet_set_bytes(TxPacket, priv->NodeUID, 16);   // NodeUID

        	g_hnode_packet_set_uint(TxPacket, priv->EndPointCount);  // End Point Count
         	g_hnode_packet_set_short(TxPacket, priv->MgmtPort);      // Mgmt Port
						
            g_hnode_packet_set_short(TxPacket, priv->ConfigPort);   // Configuration Port - 0 == configuration not supportted

    		// Save away the addressing information.
        	g_hnode_packet_set_uint(TxPacket, priv->HostAddrType);  // Address Type
    		g_hnode_packet_set_uint(TxPacket, priv->HostAddrLength);   // Address Length
            
            g_hnode_packet_set_bytes(TxPacket, priv->HostAddr, 128);  // Address string

			// Send a request for info about the provider.
            g_hnode_pktsrc_send_packet(priv->HNodeSource, TxPacket);
		break;
    }
#endif

    // Free the RX frame
    g_object_unref(Packet);

	return;
}
 
void 
hnode_x10_tx(GHNodePktSrc *sb, GHNodePacket *Packet, gpointer data)
{
    CONTEXT  *Context = (CONTEXT *)data;

    // Free the TX frame
    g_object_unref(Packet);

	return;
}

void 
hnode_switch_rx(GHNodePktSrc *sb, GHNodePacket *Packet, gpointer data)
{
    CONTEXT  *Context = (CONTEXT *)data;

    guint DataLength;
    guint PktType;
    guint ReqTag;

    GHNodePacket *TxPacket;

    DataLength = g_hnode_packet_get_data_length(Packet);
    PktType    = g_hnode_packet_get_uint(Packet);
    ReqTag     = g_hnode_packet_get_uint(Packet);

	switch( PktType )
    {
        // Request for general info about this hnode.
        case SWPKT_SWITCH_LIST_REQUEST:
            printf("Switch List Request -- Tag: 0x%x\n", ReqTag);

            TxPacket = g_hnode_packet_new();						
    
            // Set the TX address
            g_hnode_packet_clone_address(TxPacket, Packet);

	        // Check to see if this is the packet we are interested in.
            g_hnode_packet_set_uint(TxPacket, SWPKT_SWITCH_LIST_REPLY);
            g_hnode_packet_set_uint(TxPacket, ReqTag);

            g_hnode_packet_set_short(TxPacket, 4); // Number of switch records.

            g_hnode_packet_set_short(TxPacket, 0);  // Local SwitchID
    		g_hnode_packet_set_short(TxPacket, 13); // Switch Name Length
            g_hnode_packet_set_bytes(TxPacket, "x10swentryout", 13);
    		g_hnode_packet_set_short(TxPacket, 20); // Switch Description Length
            g_hnode_packet_set_bytes(TxPacket, "Outside Entry Lights", 20);
            g_hnode_packet_set_uint(TxPacket, SWINF_CAPFLAGS_ONOFF); 

            g_hnode_packet_set_short(TxPacket, 1);  // Local SwitchID
    		g_hnode_packet_set_short(TxPacket, 19); // Switch Name Length
            g_hnode_packet_set_bytes(TxPacket, "x10swentrychristmas", 19);
    		g_hnode_packet_set_short(TxPacket, 24); // Switch Description Length
            g_hnode_packet_set_bytes(TxPacket, "Outside Christmas Lights", 24);
            g_hnode_packet_set_uint(TxPacket, SWINF_CAPFLAGS_ONOFF); 

            g_hnode_packet_set_short(TxPacket, 2);  // Local SwitchID
    		g_hnode_packet_set_short(TxPacket, 15); // Switch Name Length
            g_hnode_packet_set_bytes(TxPacket, "x10appchristmas", 15);
    		g_hnode_packet_set_short(TxPacket, 23); // Switch Description Length
            g_hnode_packet_set_bytes(TxPacket, "Inside Christmas Lights", 23);
            g_hnode_packet_set_uint(TxPacket, SWINF_CAPFLAGS_ONOFF); 

            g_hnode_packet_set_short(TxPacket, 3);  // Local SwitchID
    		g_hnode_packet_set_short(TxPacket, 12); // Switch Name Length
            g_hnode_packet_set_bytes(TxPacket, "x10swentryin", 12);
    		g_hnode_packet_set_short(TxPacket, 19); // Switch Description Length
            g_hnode_packet_set_bytes(TxPacket, "Front Entry Lights", 19);
            g_hnode_packet_set_uint(TxPacket, SWINF_CAPFLAGS_ONOFF); 

			// Send a request for info about the provider.
            g_hnode_pktsrc_send_packet(Context->SwitchSource, TxPacket);
		break;

        // Request for general info about this hnode.
        case SWPKT_SWITCH_STATE_REQUEST:
            printf("Switch State Request -- Tag: 0x%x\n", ReqTag);
        break;

        case SWPKT_SWITCH_CMD_REQUEST:
        {
            guint16 SwitchID;
            guint16 SwitchCmd; 
            guint16 SwitchArg1;
            guint16 SwitchArg2;

            printf("Switch CMD Request -- Tag: 0x%x\n", ReqTag);
            
            // Read out the command data.
            SwitchID   = g_hnode_packet_get_short(Packet);  // SwitchID
            SwitchCmd  = g_hnode_packet_get_short(Packet);  // Requested Action
            SwitchArg1 = g_hnode_packet_get_short(Packet);  // Action Parameter 1
            SwitchArg2 = g_hnode_packet_get_short(Packet);  // Action Paremeter 2

            switch( SwitchCmd )
            {
                case SWINF_CMD_TURN_ON:
                    g_ilink_send_cmd(Context->ILink, 'M', (SwitchID + 1), ILINK_FUNC_ON, 1);
                break;

                case SWINF_CMD_TURN_OFF:
                    g_ilink_send_cmd(Context->ILink, 'M', (SwitchID + 1), ILINK_FUNC_OFF, 1);
                break;    
            }

	        // Build the reply packet
            TxPacket = g_hnode_packet_new();						
    
            // Set the TX address
            g_hnode_packet_clone_address(TxPacket, Packet);

            g_hnode_packet_set_uint(TxPacket, SWPKT_SWITCH_CMD_REPLY);
            g_hnode_packet_set_uint(TxPacket, ReqTag);

            g_hnode_packet_set_short(TxPacket, 0); // Cmd Result.

			// Send a request for info about the provider.
            g_hnode_pktsrc_send_packet(Context->SwitchSource, TxPacket);
        }
        break;

        case SWPKT_SWITCH_REPORT_SETTING_REQUEST:
            printf("Switch Report Setting Request -- Tag: 0x%x\n", ReqTag);            
        break;

        case SWPKT_SWITCH_SET_SETTING_REQUEST:
            printf("Switch Set Setting Request -- Tag: 0x%x\n", ReqTag);            
        break;

        case SWPKT_SWITCH_REPORT_SCHEDULE_REQUEST:
            printf("Switch Report Schedule Request -- Tag: 0x%x\n", ReqTag);            
        break;

        case SWPKT_SWITCH_SET_SCHEDULE_REQUEST:
            printf("Switch Set Schedule Request -- Tag: 0x%x\n", ReqTag);            
        break;

    }

    // Free the RX frame
    g_object_unref(Packet);

	return;
}
 
void 
hnode_switch_tx(GHNodePktSrc *sb, GHNodePacket *Packet, gpointer data)
{
    CONTEXT  *Context = (CONTEXT *)data;

    // Free the TX frame
    g_object_unref(Packet);

	return;
}

void 
hnode_cmd_tx(GILink *sb, gpointer Event, gpointer data)
{
    CONTEXT  *Context = (CONTEXT *)data;
    ILINK_CMD_RECORD RxRecord; 

    RxRecord.u.raw = (guint32) Event;

    g_print("Command Sent Callback -- House: %c  Unit: %d  FuncCode: 0x%x  Repeat: %d\n", 
                RxRecord.u.b.House, RxRecord.u.b.Unit, RxRecord.u.b.Function, RxRecord.u.b.Repeat);

	return;
}

void 
hnode_cmd_rx(GILink *sb, gpointer Event, gpointer data)
{
    CONTEXT  *Context = (CONTEXT *)data;
    ILINK_CMD_RECORD RxRecord;

    RxRecord.u.raw = (guint32) Event;

    g_print("Command Recv Callback -- House: %c  Unit: %d  FuncCode: 0x%x  Repeat: %d\n", 
                RxRecord.u.b.House, RxRecord.u.b.Unit, RxRecord.u.b.Function, RxRecord.u.b.Repeat);

    g_hnode_send_event_frame(Context->HNode, 1, 0, 4, (guint8 *)&RxRecord.u.raw);

	return;
}

int
main (AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[])
{
    GMainLoop *loop = NULL;
    CONTEXT   Context;
    GOptionContext *cmdline_context;
    GError *error = NULL;
    GHNodeAddress *AddrObj;
//    int serfd;
//	struct termios io;

    
    // Initialize the gobject type system
    g_type_init();

    // Parse any command line options.
    cmdline_context = g_option_context_new ("- a hnode implementation to control X10 devices.");
    g_option_context_add_main_entries (cmdline_context, entries, NULL); // GETTEXT_PACKAGE);
    g_option_context_parse (cmdline_context, &argc, &argv, &error);

    // Create the GLIB main loop 
    loop = g_main_loop_new (NULL, FALSE);

    Context.ILink = g_ilink_new();

    if( Context.ILink == NULL )
        exit(-1);

    // ILink intialization

    // Register the server event callback
    g_signal_connect (G_OBJECT (Context.ILink), "cmd_complete", G_CALLBACK (hnode_cmd_tx), &Context);
    g_signal_connect (G_OBJECT (Context.ILink), "async_rx", G_CALLBACK (hnode_cmd_rx), &Context);

    // Setup the ILink

    // Start up the server object
    g_ilink_start(Context.ILink, "/dev/ttyS0");


    // Start up a socket to handle raw x10 requests.
    Context.X10Source = g_hnode_pktsrc_new(PST_HNODE_UDP_ASYNCH);
    g_signal_connect(Context.X10Source, "rx_packet", G_CALLBACK(hnode_x10_rx), &Context);
    g_signal_connect(Context.X10Source, "tx_packet", G_CALLBACK(hnode_x10_tx), &Context);
    g_hnode_pktsrc_start( Context.X10Source );

    AddrObj = g_hnode_pktsrc_get_address_object(Context.X10Source);
    Context.X10Port = g_hnode_address_GetPort(AddrObj);

    // Start up a socket to handle the abstract switch interface requests.
    Context.SwitchSource = g_hnode_pktsrc_new(PST_HNODE_UDP_ASYNCH);
    g_signal_connect(Context.SwitchSource, "rx_packet", G_CALLBACK(hnode_switch_rx), &Context);
    g_signal_connect(Context.SwitchSource, "tx_packet", G_CALLBACK(hnode_switch_tx), &Context);
    g_hnode_pktsrc_start( Context.SwitchSource );

    AddrObj = g_hnode_pktsrc_get_address_object(Context.SwitchSource);
    Context.SwitchPort = g_hnode_address_GetPort(AddrObj);


    // Allocate a server object
    Context.HNode = g_hnode_new();

    if( Context.HNode == NULL )
        exit(-1);

    // HNode intialization

    // Register the server event callback
    //g_signal_connect (G_OBJECT (HNode), "state_change",
	//	      G_CALLBACK (hnode_change), NULL);

    // Setup the HNode
    g_hnode_set_version(Context.HNode, 1, 0, 0);
    g_hnode_set_uid(Context.HNode, gUID);
    g_hnode_set_name_prefix(Context.HNode, "x10Node");

    //g_hnode_enable_config_support(HNode);
    //g_signal_connect (G_OBJECT( HNode ), "config-rx", G_CALLBACK( hnode_config_rx ), NULL);

    g_hnode_set_endpoint_count(Context.HNode, 2);
    
    //guint16 EndPointIndex, guint16 AssociatedEPIndex, guint8 *MimeTypeStr, guint16 Port, guint8 MajorVersion, guint8 MinorVersion, guint16 MicroVersion)
    g_hnode_set_endpoint(Context.HNode, 0, 0, "x10-raw-interface", Context.X10Port, 1, 0, 0);	
    g_hnode_set_endpoint(Context.HNode, 1, 1, "hnode-switch-interface", Context.SwitchPort, 1, 0, 0);	

    // Start up the server object
    g_hnode_start(Context.HNode);

    // Send a command    
    //g_ilink_send_cmd(ILink, 'M', 1, ILINK_FUNC_ON, 1);

    /* Start the GLIB Main Loop */
    g_main_loop_run (loop);



    fail:
    /* Clean up */
    g_main_loop_unref (loop);

    return 0;
}
