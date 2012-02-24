//
// oscmulticast.c
// a maxmsp and puredata external for handling multicast OSC
// http://www.idmil.org/software/mappingtools
// Joseph Malloch, IDMIL 2010
// License: LGPL
//

// *********************************************************
// -(Includes)----------------------------------------------

#ifdef MAXMSP
	#include "ext.h"			// standard Max include, always required
	#include "ext_obex.h"		// required for new style Max object
	#include "ext_dictionary.h"
	#include "jpatcher_api.h"
#else
	#include "m_pd.h"
    #define A_SYM A_SYMBOL
#endif
#include <stdio.h>
#include <string.h>
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
static void *oscmulticast_new(t_symbol *s, int argc, t_atom *argv);
static void oscmulticast_free(t_oscmulticast *x);
static void oscmulticast_anything(t_oscmulticast *x, t_symbol *s, int argc, t_atom *argv);
static void oscmulticast_poll(t_oscmulticast *x);
static int oscmulticast_handler(const char *path, const char *types, lo_arg ** argv,
                         int argc, void *data, void *user_data);
#ifdef MAXMSP
	static void oscmulticast_assist(t_oscmulticast *x, void *b, long m, long a, char *s);
#endif

static const char *maxpd_atom_get_string(t_atom *a);
static void maxpd_atom_set_string(t_atom *a, const char *string);
static void maxpd_atom_set_int(t_atom *a, int i);
static double maxpd_atom_get_float(t_atom *a);
static void maxpd_atom_set_float(t_atom *a, float d);

// *********************************************************
// -(global class pointer variable)-------------------------
static void *oscmulticast_class;

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
    const char *group = NULL;
    char port[0], address[64];

#ifdef MAXMSP
    if (x = object_alloc(oscmulticast_class)) {
        x->om_outlet = outlet_new((t_object *)x, 0);
#else
    if (x = (t_oscmulticast *) pd_new(oscmulticast_class)) {
        x->om_outlet = outlet_new(&x->ob, 0);
#endif

        if (argc < 4) {
            post("Not enough arguments!\n");
            return NULL;
        }
        for (i = 0; i < argc; i++) {
            if(strcmp(maxpd_atom_get_string(argv+i), "@group") == 0) {
                if ((argv+i+1)->a_type == A_SYM) {
                    group = maxpd_atom_get_string(argv+i+1);
                    i++;
                }
            }
            else if (strcmp(maxpd_atom_get_string(argv+i), "@port") == 0) {
                if ((argv+i+1)->a_type == A_FLOAT) {
                    snprintf(port, 10, "%i", (int)maxpd_atom_get_float(argv+i+1));
                    i++;
                }
#ifdef MAXMSP
                else if ((argv+i+1)->a_type == A_LONG) {
                    snprintf(port, 10, "%i", (int)atom_getlong(argv+i+1));
                    i++;
                }
#endif
            }
        }

        if (group && port) {
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
    }
	return (x);
}

// *********************************************************
// -(free)--------------------------------------------------
void oscmulticast_free(t_oscmulticast *x)
{
    if (x->om_clock) {
        clock_unset(x->om_clock);	// Remove clock routine from the scheduler
        clock_free(x->om_clock);		// Frees memory used by clock
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
#endif
            case A_SYM:
                lo_message_add_string(m, maxpd_atom_get_string(argv + i));
                break;
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
                maxpd_atom_set_int(x->buffer+j, argv[i]->i);
				j++;
                break;
            case 'h':
                maxpd_atom_set_int(x->buffer+j, argv[i]->h);
				j++;
                break;
            case 'f':
                maxpd_atom_set_float(x->buffer+j, argv[i]->f);
				j++;
                break;
            case 'd':
                maxpd_atom_set_float(x->buffer+j, (float)argv[i]->d);
				j++;
                break;
            case 's':
                maxpd_atom_set_string(x->buffer+j, (const char *)&argv[i]->s);
				j++;
                break;
            case 'S':
                maxpd_atom_set_string(x->buffer+j, (const char *)&argv[i]->s);
				j++;
                break;
            case 'c':
                snprintf(my_string, 2, "%c", argv[i]->c);
                maxpd_atom_set_string(x->buffer+j, (const char *)my_string);
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

// *********************************************************
// some helper functions for abtracting differences
// between maxmsp and puredata 

const char *maxpd_atom_get_string(t_atom *a)
{
#ifdef MAXMSP
    return atom_getsym(a)->s_name;
#else
    return (a)->a_w.w_symbol->s_name;
#endif
}

void maxpd_atom_set_string(t_atom *a, const char *string)
{
#ifdef MAXMSP
    atom_setsym(a, gensym((char *)string));
#else
    SETSYMBOL(a, gensym(string));
#endif
}

void maxpd_atom_set_int(t_atom *a, int i)
{
#ifdef MAXMSP
    atom_setlong(a, (long)i);
#else
    SETFLOAT(a, (double)i);
#endif
}

double maxpd_atom_get_float(t_atom *a)
{
    return (double)atom_getfloat(a);
}

void maxpd_atom_set_float(t_atom *a, float d)
{
#ifdef MAXMSP
    atom_setfloat(a, d);
#else
    SETFLOAT(a, d);
#endif
}
