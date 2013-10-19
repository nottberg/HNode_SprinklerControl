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
//#include "ilink-obj.h"
#include "mcp23008-obj.h"

#include "REST/REST.hpp"

#include "SwitchManager.hpp"
#include "SwitchResource.hpp"

#include "ZoneManager.hpp"

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
  { "device", 'd', 0, G_OPTION_ARG_INT, &wait_time, "The device path for the i2c port.", "N" },
  { "print", 0, 0, G_OPTION_ARG_INT, &event_period, "Turn on printing of rx and tx to standard out", "N" },
  { NULL }
};

typedef struct x10NodeContext
{
    //GILink    *ILink;
    //GMCP23008 *gpio;
    GHNode    *HNode;

    GHNodePktSrc    *SwitchSource;

    guint16  SwitchPort;

    RESTDaemon   Rest;
    //RESTResource controller;
    RESTResource toggles;
    //RESTResource zones;
    //RESTResource zone;

    ZoneManager   zoneManager;

    SwitchManager switchManager;

    SwitchListResource switchListResource;
    SwitchResource switchResource;
}CONTEXT;

static struct termios saved_io;

void 
hnode_switch_rx(GHNodePktSrc *sb, GHNodePacket *Packet, gpointer data)
{
    CONTEXT  *Context = (CONTEXT *)data;

    SwitchManager *swmgr = &Context->switchManager;

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

            g_hnode_packet_set_short(TxPacket, swmgr->getSwitchCount()); // Number of switch records.

            for( int index = 0; index < swmgr->getSwitchCount(); index++ )
            {
                std::string tmpStr;
                unsigned int tmpLen;
                Switch *swObj = swmgr->getSwitchByIndex( index );

                g_hnode_packet_set_short(TxPacket, index);  // Local SwitchID

                tmpStr = swObj->getName();
                tmpLen = tmpStr.size();
                g_hnode_packet_set_short(TxPacket, tmpLen); // Switch Name Length
                g_hnode_packet_set_bytes(TxPacket, (guint8*)tmpStr.c_str(), tmpLen);

                tmpStr = swObj->getDescription();
                tmpLen = tmpStr.size();
                g_hnode_packet_set_short(TxPacket, tmpLen); // Switch Name Length
                g_hnode_packet_set_bytes(TxPacket, (guint8*)tmpStr.c_str(), tmpLen);

                g_hnode_packet_set_uint(TxPacket, SWINF_CAPFLAGS_ONOFF); 
            }

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
                {
                     Switch *swObj = swmgr->getSwitchByIndex( SwitchID );
                     if( swObj )
                         swObj->setStateOn();
                    //g_mcp23008_set_pin_state(Context->gpio, SwitchID, 1 );
                    //g_ilink_send_cmd(Context->ILink, 'M', (SwitchID + 1), ILINK_FUNC_ON, 1);
                }
                break;

                case SWINF_CMD_TURN_OFF:
                {
                     Switch *swObj = swmgr->getSwitchByIndex( SwitchID );
                     if( swObj )
                         swObj->setStateOff();
                    //g_mcp23008_set_pin_state(Context->gpio, SwitchID, 0 );
                    //g_ilink_send_cmd(Context->ILink, 'M', (SwitchID + 1), ILINK_FUNC_OFF, 1);
                }
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

#if 0
void 
hnode_cmd_tx(GILink *sb, gpointer Event, gpointer data)
{
    CONTEXT  *Context = (CONTEXT *)data;
//    ILINK_CMD_RECORD RxRecord; 

//    RxRecord.u.raw = (guint32) Event;

//    g_print("Command Sent Callback -- House: %c  Unit: %d  FuncCode: 0x%x  Repeat: %d\n", 
//                RxRecord.u.b.House, RxRecord.u.b.Unit, RxRecord.u.b.Function, RxRecord.u.b.Repeat);

	return;
}

void 
hnode_cmd_rx(GILink *sb, gpointer Event, gpointer data)
{
    CONTEXT  *Context = (CONTEXT *)data;
//    ILINK_CMD_RECORD RxRecord;
//
//    RxRecord.u.raw = (guint32) Event;
//
//    g_print("Command Recv Callback -- House: %c  Unit: %d  FuncCode: 0x%x  Repeat: %d\n", 
//                RxRecord.u.b.House, RxRecord.u.b.Unit, RxRecord.u.b.Function, RxRecord.u.b.Repeat);
//
//    g_hnode_send_event_frame(Context->HNode, 1, 0, 4, (guint8 *)&RxRecord.u.raw);

	return;
}
#endif

int
hnode_start_rest_daemon(CONTEXT *Context)
{
    // Load the configuration for the switches
    Context->switchManager.loadConfiguration();

    // Init the REST resources.
    Context->switchListResource.setSwitchManager( &Context->switchManager );
    Context->switchResource.setSwitchManager( &Context->switchManager );

    //controller.setURLPattern( "/irrigation", REST_RMETHOD_GET );
    //rest.registerResource( &controller );

    //Context->toggles.setURLPattern( "/irrigation/toggles", REST_RMETHOD_GET );
    Context->Rest.registerResource( &(Context->switchListResource) );
    Context->Rest.registerResource( &(Context->switchResource) );

    //zones.setURLPattern( "/irrigation/zones", (REST_RMETHOD_T)(REST_RMETHOD_GET | REST_RMETHOD_POST) );
    //rest.registerResource( &zones );

    //zone.setURLPattern( "/irrigation/zones/{zoneid}", (REST_RMETHOD_T)(REST_RMETHOD_GET | REST_RMETHOD_PUT | REST_RMETHOD_DELETE) ); 
    //rest.registerResource( &zone );

    Context->Rest.start();
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
    cmdline_context = g_option_context_new ("- a hnode implementation for raspberry pi sprinkler controller.");
    g_option_context_add_main_entries (cmdline_context, entries, NULL); // GETTEXT_PACKAGE);
    g_option_context_parse (cmdline_context, &argc, &argv, &error);

    // Create the GLIB main loop 
    loop = g_main_loop_new (NULL, FALSE);

    //Context.ILink = g_ilink_new();

    //if( Context.ILink == NULL )
    //    exit(-1);

    // ILink intialization
    //Context.gpio = g_mcp23008_new();

    //if( Context.gpio == NULL )
    //    exit(-1);

    //g_mcp23008_set_i2c_address( Context.gpio, 0, 0x20 );

    //g_mcp23008_start( Context.gpio );

    // Set pins 0, 1, 2 as outputs
    //g_mcp23008_set_port_mode( Context.gpio, (MCP23008_PM_PIN0_OUT | MCP23008_PM_PIN1_OUT | MCP23008_PM_PIN2_OUT) );
    
    // Set pin 3 to input with the pullup resistor enabled
    //g_mcp23008_set_port_pullup( Context.gpio, (MCP23008_PU_PIN3_ON) );

    // Read pin 3 and display the results
    //printf( "%d: %x\n", 3, g_mcp23008_check_pin_state( Context.gpio, 3 ) );

    // Register the server event callback
    //g_signal_connect (G_OBJECT (Context.ILink), "cmd_complete", G_CALLBACK (hnode_cmd_tx), &Context);
    //g_signal_connect (G_OBJECT (Context.ILink), "async_rx", G_CALLBACK (hnode_cmd_rx), &Context);

    // Setup the ILink

    // Start up the server object
    //g_ilink_start(Context.ILink, "/dev/ttyS0");

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
    g_hnode_set_name_prefix(Context.HNode, (guint8*)"SprinklerControl");

    //g_hnode_enable_config_support(HNode);
    //g_signal_connect (G_OBJECT( HNode ), "config-rx", G_CALLBACK( hnode_config_rx ), NULL);

    g_hnode_set_endpoint_count(Context.HNode, 2);
    
    //guint16 EndPointIndex, guint16 AssociatedEPIndex, guint8 *MimeTypeStr, guint16 Port, guint8 MajorVersion, guint8 MinorVersion, guint16 MicroVersion)
    g_hnode_set_endpoint(Context.HNode, 0, 0, (guint8*)"hnode-switch-interface", Context.SwitchPort, 1, 0, 0);	
    g_hnode_set_endpoint(Context.HNode, 1, 1, (guint8*)"hnode-irrigation-rest", 8888, 1, 0, 0);	

    // Fire up the rest daemon
    hnode_start_rest_daemon( &Context );

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
