//
// oscmulticast.c
// a maxmsp external for passing multicast OSC
// http://www.idmil.org/software/mappingtools
// Joseph Malloch, IDMIL 2010
// LGPL
//

#ifndef PD
#define MAXMSP
#endif

// *********************************************************
// -(Includes)----------------------------------------------

#ifdef MAXMSP
	#include "ext.h"			// standard Max include, always required
	#include "ext_obex.h"		// required for new style Max object
	#include "ext_dictionary.h"
	#include "jpatcher_api.h"
#else
	#include "m_pd.h"
#endif
#include <stdio.h>
#include "lo/lo.h"

#define INTERVAL 1
#define MAXSIZE 256

// *********************************************************
// -(object struct)-----------------------------------------
typedef struct _oscmulticast 
{
	t_object ob;
    void *om_outlet;
    lo_server om_server;
    lo_address om_address;
    void *om_clock;          // pointer to clock object
	t_atom buffer[MAXSIZE];
} t_oscmulticast;

// *********************************************************
// -(function prototypes)-----------------------------------
void *oscmulticast_new(t_symbol *s, int argc, t_atom *argv);
void oscmulticast_free(t_oscmulticast *x);
void oscmulticast_anything(t_oscmulticast *x, t_symbol *s, int argc, t_atom *argv);
void oscmulticast_poll(t_oscmulticast *x);
int oscmulticast_handler(const char *path, const char *types, lo_arg ** argv,
                         int argc, void *data, void *user_data);
#ifdef MAXMSP
	void oscmulticast_assist(t_oscmulticast *x, void *b, long m, long a, char *s);
#endif

// *********************************************************
// -(global class pointer variable)-------------------------
void *oscmulticast_class;

// *********************************************************
// -(main)--------------------------------------------------
#ifdef MAXMSP
	int main(void)
	{	
		t_class *c;
		c = class_new("oscmulticast", (method)oscmulticast_new, (method)oscmulticast_free, 
					  (long)sizeof(t_oscmulticast), 0L, A_GIMME, 0);
		class_addmethod(c, (method)oscmulticast_assist,         "assist",   A_CANT,     0);
		class_addmethod(c, (method)oscmulticast_anything,       "anything", A_GIMME,    0);
		class_register(CLASS_BOX, c); /* CLASS_NOBOX */
		oscmulticast_class = c;
		return 0;
	}
#else
	int oscmulticast_setup(void)
	{
		t_class *c;
		c = class_new(gensym("oscmulticast"), (t_newmethod)oscmulticast_new, (t_method)oscmulticast_free, 
					  (long)sizeof(t_oscmulticast), 0L, A_GIMME, 0);
		class_addanything(c, (t_method)oscmulticast_anything);
		oscmulticast_class = c;
		return 0;
	}
#endif

// *********************************************************
// -(new)---------------------------------------------------
void *oscmulticast_new(t_symbol *s, int argc, t_atom *argv)
{
	t_oscmulticast *x = NULL;
    int i;
    char *group = NULL, port[10], address[64];
    
    if (argc < 4) {
        post("Not enough arguments!\n");
        return NULL;
    }
#ifdef MAXMSP
    for (i = 0; i < argc; i++) {
        if(strcmp(atom_getsym(argv+i)->s_name, "@group") == 0) {
            if ((argv+i+1)->a_type == A_SYM) {
                group = strdup(atom_getsym(argv+i+1)->s_name);
                i++;
            }
        }
        else if (strcmp(atom_getsym(argv+i)->s_name, "@port") == 0) {
            if ((argv+i+1)->a_type == A_LONG) {
                snprintf(port, 10, "%i", (int)atom_getlong(argv+i+1));
                i++;
            }
        }
    }
	if (x = object_alloc(oscmulticast_class))
        x->om_outlet = outlet_new((t_object *)x, 0);
	else
		return 0;
        
#else
	for (i = 0; i < argc; i++) {
        if(strcmp((argv+i)->a_w.w_symbol->s_name, "@group") == 0) {
            if ((argv+i+1)->a_type == A_SYMBOL) {
                group = strdup((argv+i+1)->a_w.w_symbol->s_name);
                i++;
            }
        }
        else if (strcmp((argv+i)->a_w.w_symbol->s_name, "@port") == 0) {
            if ((argv+i+1)->a_type == A_FLOAT) {
                snprintf(port, 10, "%i", (int)atom_getfloat(argv+i+1));
                i++;
            }
        }
    }    
    if (x = (t_oscmulticast *) pd_new(oscmulticast_class))
        x->om_outlet = outlet_new(&x->ob, 0);
	else
		return 0;
#endif
        
	if (&group && port) {
		snprintf(address, 64, "osc.udp://%s:%s", group, port);
		x->om_address = lo_address_new_from_url(address);
		lo_address_set_ttl(x->om_address, 1);
		
		x->om_server = lo_server_new_multicast(group, port, 0);
		lo_server_add_method(x->om_server, NULL, NULL, oscmulticast_handler, x);
	}
	
#ifdef MAXMSP
	x->om_clock = clock_new(x, (method)oscmulticast_poll);	// Create the timing clock
#else
	x->om_clock = clock_new(x, (t_method)oscmulticast_poll);
#endif
	clock_delay(x->om_clock, INTERVAL);  // Set clock to go off after delay

	return (x);
}

// *********************************************************
// -(free)--------------------------------------------------
void oscmulticast_free(t_oscmulticast *x)
{
    if (x->om_clock) {
        clock_unset(x->om_clock);	// Remove clock routine from the scheduler
        clock_free(x->om_clock);		// Frees memeory used by clock
    }
    if (x->om_server) {
        lo_server_free(x->om_server);
    }
    if (x->om_address) {
        lo_address_free(x->om_address);
    }
}

// *********************************************************
// -(inlet/outlet assist - maxmsp only)---------------------
#ifdef MAXMSP
void oscmulticast_assist(t_oscmulticast *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
		sprintf(s, "OSC to be sent to multicast bus");
	} 
	else {	// outlet
        sprintf(s, "OSC from multicast bus");
	}
}
#endif

// *********************************************************
// -(anything)----------------------------------------------
void oscmulticast_anything(t_oscmulticast *x, t_symbol *s, int argc, t_atom *argv)
{
    lo_message m = lo_message_new();
    if (!m) {
        post("lo_message_new() error");
        return;
    }
    
    int i;
    for (i=0; i<argc; i++)
    {
        switch ((argv + i)->a_type)
        {
			case A_FLOAT:
                lo_message_add_float(m, atom_getfloat(argv + i));
                break;
#ifdef MAXMSP
            case A_LONG:
                lo_message_add_int32(m, (int)atom_getlong(argv + i));
                break;
            case A_SYM:
                lo_message_add_string(m, (atom_getsym(argv + i)->s_name));
                break;
#else
			case A_SYMBOL:
				lo_message_add_string(m, (argv + i)->a_w.w_symbol->s_name);
#endif
        }
    }
    //set timetag?
    
    lo_send_message(x->om_address, s->s_name, m);
    lo_message_free(m);
}

// *********************************************************
// -(poll libmapper)----------------------------------------
void oscmulticast_poll(t_oscmulticast *x)
{
    int count = 0;
    
    while (count < 10 && lo_server_recv_noblock(x->om_server, 0)) {
        count++;
    }
	clock_delay(x->om_clock, INTERVAL);  // Set clock to go off after delay
}

// *********************************************************
// -(OSC handler)-------------------------------------------
int oscmulticast_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{
    t_oscmulticast *x = (t_oscmulticast *)user_data;
    int i, j;
    char my_string[2];
    
    j=0;
    
    if (!x->buffer) {
        post("Error receiving message!");
        return 0;
    }
    
    if (argc > MAXSIZE) {
        post("Truncating received message to 256 elements!");
        argc = MAXSIZE;
    }
    
    for (i=0; i<argc; i++)
    {
        switch (types[i])
        {
            case 'i':
#ifdef MAXMSP
                atom_setlong(x->buffer+j, (long)argv[i]->i);
#else
				SETFLOAT(x->buffer+j, (float)argv[i]->i);
#endif
				j++;
                break;
            case 'h':
#ifdef MAXMSP
                atom_setlong(x->buffer+j, (long)argv[i]->h);
#else
				SETFLOAT(x->buffer+j, (float)argv[i]->h);
#endif
				j++;
                break;
            case 'f':
#ifdef MAXMSP
                atom_setfloat(x->buffer+j, argv[i]->f);
#else
				SETFLOAT(x->buffer+j, argv[i]->f);
#endif
				j++;
                break;
            case 'd':
#ifdef MAXMSP
                atom_setfloat(x->buffer+j, (float)argv[i]->d);
#else
				SETFLOAT(x->buffer+j, (float)argv[i]->d);
#endif
				j++;
                break;
            case 's':
#ifdef MAXMSP
                atom_setsym(x->buffer+j, gensym(&argv[i]->s));
#else
				SETSYMBOL(x->buffer+j, gensym(&argv[i]->s));
#endif
				j++;
                break;
            case 'S':
#ifdef MAXMSP
                atom_setsym(x->buffer+j, gensym(&argv[i]->s));
#else
				SETSYMBOL(x->buffer+j, gensym(&argv[i]->s));
#endif
				j++;
                break;
            case 'c':
                snprintf(my_string, 2, "%c", argv[i]->c);
#ifdef MAXMSP
                atom_setsym(x->buffer+j, gensym(my_string));
#else
				SETSYMBOL(x->buffer+j, gensym(my_string));
#endif
				j++;
                break;
            case 't':
                //output timetag from a second outlet?
                break;
        }
    }
    outlet_anything(x->om_outlet, gensym((char *)path), j, x->buffer);
    return 0;
}