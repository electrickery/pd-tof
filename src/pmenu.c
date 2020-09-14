/* pmenu widget for PD by Thomas Ouellet Fredericks                    *
 * Based on:                                                           *
 *  [bbogart/popup] by Ben Bogart                                      *
 *  and [ggee/button] from GGEE by Guenter Geiger                      *

 * pmenu was previously licensed under GPL, but Thomas allowed me to   *
 * re-license it under the 3-clause BSD license. This brings it more   *
 * in line with the original ggee/button code. The code from           *
 * bbogart/popup is still GPL licensed.                                *
 * fjkraan@xs4all.nl. 2016-06-14                                       *
 */

/* CHANGES
 
 0.32 changed %x to %lx. This failed on MaxOSX-64-bit (fjk, 2016-10-30).
 0.31 %p or %x in sprintf and sysvgui do not seem to give the same result, 
      so generate the name in sprintf and feed it as a string to sysvgui
 0.3 Added the possibility of adding anything (instead of only symbols)
 0.3 Split the output message into two outlets

 */


/* Append " x " to the following line to show debugging messages */
#define DEBUG(x)



#include <m_pd.h>
#include <g_canvas.h>
#include <stdio.h>
#include <string.h>


static char pmenu_buffer[MAXPDSTRING];




typedef struct _pmenu
{
     t_object x_obj;

     t_glist * x_glist;

     t_symbol*  callback;
	
     int current_selection;
     	 
     t_symbol* bg_color;
     t_symbol* fg_color;
     t_symbol* hi_color;
     t_symbol* co_color;
    
     t_atom* options;
     int     options_memory;
	 int     options_count;
     int     max_column_height;
   
      int indicator;
      int focusing;
	
	t_outlet* outlet1;
	t_outlet* outlet2;
   
} t_pmenu;

#include "pmenu_w.h"

static void pmenu_output(t_pmenu* x)
{
	/*
  t_atom atoms[2];
  SETFLOAT(atoms,x->current_selection);
  //SETSYMBOL(atoms+1,x->x_options[x->current_selection]);
  atoms[1] = *(x->options+x->current_selection);
  outlet_list(x->x_obj.ob_outlet, &s_list, 2, atoms);
	*/
	outlet_float(x->outlet1, x->current_selection);
	// outlet_anything(t_outlet *x, t_symbol *s, int argc, t_atom *argv)
	if ( (x->options+x->current_selection)->a_type == A_SYMBOL ) {
		outlet_symbol(x->outlet2, atom_getsymbol(x->options+x->current_selection));
	} else {
		outlet_float(x->outlet2, atom_getfloat(x->options+x->current_selection));

	}
	
	
				 
  //if ( x->send != x->s_empty && x->send->s_thing) pd_forwardmess(x->send->s_thing, 2,atoms);

}


static void pmenu_callback(t_pmenu* x, t_floatarg f)
{
  //DEBUG(post("output start");)
  x->current_selection= (int)f;
  //pmenu_w_text(x,x->x_options[x->current_selection]);
  pmenu_output(x);
  
}


static void pmenu_clear(t_pmenu* x) {
	x->options_count = 0;
	x->current_selection = -1;
	pmenu_w_clear(x);
}



static void pmenu_add(t_pmenu* x, t_symbol *s, int argc, t_atom *argv) {
		
	// resize the options-array if it is too small
	if((argc + x->options_count) > x->options_memory){
          
          x->options = resizebytes( x->options, x->options_memory*sizeof(*(x->options)), 
				(argc + x->options_count+10)*sizeof(*(x->options)));
          x->options_memory=argc + x->options_count+10;
    }
	
	int i;
	t_symbol* label;
	for  ( i=0;i<argc;i++) {
		// Copy atom
		*(x->options+x->options_count) = *(argv+i);
		
		if ((argv+i)->a_type==A_SYMBOL) {
			
		    label = atom_getsymbol(argv+i);
		   
		} else {
		
				atom_string(argv+i, pmenu_buffer, MAXPDSTRING);
				label = gensym(pmenu_buffer);
			
		}
		DEBUG(post("adding option: %s",label->s_name);)
		pmenu_w_additem( x,label,x->options_count);
		x->options_count = x->options_count + 1;
		
	}
	
    
}



// function to change the colors

static void pmenu_colors(t_pmenu* x, t_symbol* s, int argc, t_atom* argv)
{
    
	DEBUG(post("bgcolour start");)
    
    if ( argc && argv->a_type==A_SYMBOL && atom_getsymbol(argv)==gensym("default")) {
		x->bg_color = gensym("grey90");
		x->fg_color = gensym("black");
		x->hi_color = gensym("grey95");
		x->co_color = gensym("black");
	} else {
    
		if (argc > 4) argc = 4;
		switch (argc) {
			case 4: if ((argv+3)->a_type==A_SYMBOL) x->co_color = atom_getsymbol(argv+3);
			case 3: if ((argv+2)->a_type==A_SYMBOL) x->hi_color = atom_getsymbol(argv+2);
			case 2: if ((argv+1)->a_type==A_SYMBOL) x->fg_color = atom_getsymbol(argv+1);
			case 1: if ((argv)->a_type==A_SYMBOL) x->bg_color = atom_getsymbol(argv);
			break;
		}
    }
    
    pmenu_w_apply_colors(x);
	
}


static int pmenu_set_float(t_pmenu* x, t_floatarg item) {
	
	    
	int i=(int)item;
	if( (i < x->options_count) && (i >= 0)) {
		x->current_selection = i;
		//if(pmenu_w_is_visible(x)) pmenu_w_text(x,x->x_options[x->current_selection]);
		pmenu_w_activate(x);
         return 1;
	} else {
		x->current_selection = -1;
		pmenu_w_activate(x);
        return 0;
   }
	
}

/* Function to select a menu option by inlet */
static void pmenu_float(t_pmenu* x, t_floatarg item)
{
	DEBUG(post("iselect start");)
        
	if ( pmenu_set_float(x,item) ) {
		pmenu_output(x);
	} else {
		
		//pd_error(x,"pmenu: expecting value between 0 and %i",x->x_num_options);
	}

	DEBUG(post("iselect end");)
}

static int pmenu_set_symbol(t_pmenu* x, t_symbol *s) {
	

	int i;
        
	/* Compare inlet symbol to each option */
	for(i=0; i < x->options_count; i++) {
	  if( (x->options+i)->a_type==A_SYMBOL && atom_getsymbol(x->options+i)->s_name == s->s_name) {
		    x->current_selection = i;
			//if(pmenu_w_is_visible(x)) pmenu_w_text(x,s);
            pmenu_w_activate(x);
	        return 1;
       }
	}
	x->current_selection = -1;
	pmenu_w_activate(x);
	return 0;
}


/* Function to choose value via symbol name */
static void pmenu_symbol(t_pmenu* x, t_symbol *s)
{
	if(pmenu_set_symbol(x,s)) {
	   pmenu_output(x);
  } else {
	   post("pmenu: '%s' is not an available option.", s->s_name);
  }
	
}

/* Function to choose value via name/index but without outputting it*/
static void pmenu_set(t_pmenu* x, t_symbol *S, int argc, t_atom*argv)
{
  if(!argc)return;

  if(argv->a_type==A_FLOAT) {
      pmenu_set_float(x,atom_getfloat(argv));
    } else if(argv->a_type==A_SYMBOL) {
		pmenu_set_symbol(x,atom_getsymbol(argv));
     } else {
      pd_error(x, "pmenu: can only 'set' symbols or floats");
    }
}

static void pmenu_max_column_height(t_pmenu* x, t_floatarg item)
{
    x->max_column_height = (item >= 0.f) ? (int)item : 0;
    DEBUG(post("x->max_column_height: %d", x->max_column_height);)
}


static t_class *pmenu_class;



static void pmenu_free(t_pmenu*x)
{
	pmenu_w_menu(x,DESTROY);

	
   freebytes(x->options, x->options_memory * sizeof(*(x->options)));
	
   pd_unbind(&x->x_obj.ob_pd, x->callback);
   
   //if ( x->receive != x->s_empty ) pd_unbind(&x->x_obj.ob_pd, x->receive);
}


static void *pmenu_new(t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("pmenu new start");)

    t_pmenu *x = (t_pmenu *)pd_new(pmenu_class);


    x->current_selection = -1;

    x->options_memory=10;
    x->options=getbytes(sizeof(*(x->options)) * x->options_memory);
	//x->av = getbytes(x->mem_size * sizeof(*(x->av)));
    x->max_column_height = 0;
 
    x->options_count = 0 ;
  
    x->indicator = 1;
    x->focusing = 1;
  
    
    // These should match the default colors in pmenu_colors
    x->bg_color = gensym("grey90");
    x->fg_color = gensym("black");
    x->hi_color = gensym("grey95");
    x->co_color = gensym("black");
      
	
    if (argc > 6) argc = 6;
    switch(argc){ 
    case 6: if ((argv+5)->a_type==A_SYMBOL) x->co_color = atom_getsymbol(argv+5);	
	case 5: if ((argv+4)->a_type==A_SYMBOL) x->hi_color = atom_getsymbol(argv+4);	
	case 4: if ((argv+3)->a_type==A_SYMBOL) x->fg_color = atom_getsymbol(argv+3);	
	case 3: if ((argv+2)->a_type==A_SYMBOL) x->bg_color = atom_getsymbol(argv+2);
	case 2: if ((argv+1)->a_type==A_FLOAT) x->focusing = atom_getfloat(argv+1);
	case 1: if ((argv)->a_type==A_FLOAT) x->indicator = atom_getfloat(argv);
      break;
    }

      /* Bind the recieve "pmenu%p" to the widget outlet*/
      sprintf(pmenu_buffer,"pmenu%p",x);
	 
	
      x->callback = gensym(pmenu_buffer);
      pd_bind(&x->x_obj.ob_pd, x->callback);

      /* define proc in tcl/tk where "pmenu%p" is the receive, "callback" is the method, and "$index" is an argument. */
    //sys_vgui("proc select%x {index} {\n pdsend \"pmenu%p callback $index \"\n }\n",x,x); 
    sys_vgui("proc select%lx {index} {\n pdsend \"%s callback $index \"\n }\n",x,pmenu_buffer); 

    x->outlet1 = outlet_new(&x->x_obj, &s_float);
	x->outlet2 = outlet_new(&x->x_obj, &s_list);

   
   pmenu_w_menu(x,CREATE);
   pmenu_w_apply_colors(x);
  

DEBUG(post("pmenu new end");)

    return (x);
}

void pmenu_setup(void) {

    DEBUG(post("setup start");)

      pmenu_class = class_new(gensym("pmenu"), (t_newmethod)pmenu_new, (t_method)pmenu_free,
				sizeof(t_pmenu),0,A_GIMME,0);
				
	class_addbang(pmenu_class,	(t_method)pmenu_w_pop);
				
	class_addmethod(pmenu_class, (t_method)pmenu_callback,
								  gensym("callback"),A_DEFFLOAT,0);
	
	class_addmethod(pmenu_class, (t_method)pmenu_add,
								  gensym("add"), A_GIMME,0);
	
	class_addmethod(pmenu_class, (t_method)pmenu_clear,
								  gensym("clear"), 0);
						  
	class_addmethod(pmenu_class, (t_method)pmenu_colors,
								  gensym("colors"),A_GIMME,0);
	
        class_addmethod(pmenu_class, (t_method)pmenu_set,
                                    gensym("set"),A_GIMME,0);
	
	class_addsymbol(pmenu_class, (t_method)pmenu_symbol);

	class_addfloat(pmenu_class, (t_method)pmenu_float);

	class_addmethod(pmenu_class, (t_method)pmenu_max_column_height,
								  gensym("max_column_height"), A_DEFFLOAT, 0);
	


	post("pmenu v0.32 by tof");
}


