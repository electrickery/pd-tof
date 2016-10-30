/* imagebang widget for PD by Thomas Ouellet Fredericks                *
 * Based on:                                                           *
 *   [ggee/image] by Guenter Geiger and                                *
 *   [moonlib/image] by Antoine Rousseau (GPL-license)                 * 

 * This code is licensed under the 3-clause BSD license with an        *
 * exception for the code from  moonlib/image                          *
 * fjkraan@xs4all.nl. 2016-06-14                                       */
 
#define VERSION 0.6 

/* Version 0.5 new behaviour:                                          *
 * * default images instead of object load fail when image files are   *
 *   not found                                                         *
 * * like bang: change send and receive symbol                         *
 * * like bang: change flash time                                      *
 * * override flash time  indefinitly                                  *
 */
 
#include <m_pd.h>
#include <g_canvas.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* Append " x " to the following line to show debugging messages */
#define DEBUG(x)

#define CLOCKFLASH 250
#define CLOCKBRK   50


/* ------------------------ imagebang ----------------------------- */

static t_class *imagebang_class;
t_widgetbehavior imagebang_widgetbehavior;


typedef struct _imagebang
{
     t_object x_obj;
     t_glist * glist;
     int width;
     int height;
     t_symbol*  image_a;
     t_symbol*  image_b;
     t_symbol* receive;
     t_symbol* send;
     t_clock* clock_flash;
     t_clock* clock_brk;
     int clockflash;
     int clockbrk;
     int 		flashing;
	 t_outlet* outlet;
} t_imagebang;


static void imagebang_bang(t_imagebang *x)
{
	
	t_glist* glist = glist_getcanvas(x->glist);
    if(x->flashing) {
		sys_vgui(".x%lx.c itemconfigure %lximage -image %lx_imagebang \n", glist, x,x->image_a);
        clock_delay(x->clock_brk, x->clockbrk);
        //x->flashed = 1;
    } else  {
		sys_vgui(".x%lx.c itemconfigure %lximage -image %lx_imagebang \n", glist, x,x->image_b);
        x->flashing = 1;
        
    }
    clock_delay(x->clock_flash, x->clockflash);
    
    outlet_bang(x->outlet);
    
    if(x->send && x->send->s_thing ) pd_bang(x->send->s_thing);
    
}

static void imagebang_flash_timeout(t_imagebang *x)
{
	t_glist* glist = glist_getcanvas(x->glist);
    x->flashing = 0;
    sys_vgui(".x%lx.c itemconfigure %lximage -image %lx_imagebang \n", glist, x,x->image_a);
    
}

static void imagebang_brk_timeout(t_imagebang *x)
{
	t_glist* glist = glist_getcanvas(x->glist);
    x->flashing = 1;
    sys_vgui(".x%lx.c itemconfigure %lximage -image %lx_imagebang \n", glist, x,x->image_b);
    
}


/* widget helper functions */

static const char* imagebang_get_filename(t_imagebang *x,char *file) {
	static char fname[MAXPDSTRING];
	char *bufptr;
	int fd;
	
	fd=open_via_path(canvas_getdir(glist_getcanvas(x->glist))->s_name, 
	    file, "",fname, &bufptr, MAXPDSTRING, 1);
	if(fd>0){
	  	fname[strlen(fname)]='/';
	  	DEBUG(post("image file: %s",fname);)
	  	close(fd);
	  	return fname;
	} else {
		return 0;
	}
}

static int imagebang_click(t_imagebang *x, struct _glist *glist,
    int xpos, int ypos, int shift, int alt, int dbl, int doit) {
		//DEBUG(post("x:%i y:%i dbl:%i doit:%i",xpos,ypos,dbl,doit);)
		if ( doit) imagebang_bang(x) ;
		
		return (1);
	}




static void imagebang_drawme(t_imagebang *x, t_glist *glist, int firsttime) {
     if (firsttime) {	
		 
		 DEBUG(post("Rendering: \n   %lx_imagebang:%s \n   %lx_imagebang:%s",
                                              x->image_a,  x->image_a->s_name, x->image_b, x->image_b->s_name);)
		 
		sys_vgui(".x%lx.c create image %d %d -anchor nw -image %lx_imagebang -disabledimage %lx_imagebang -tags %lximage\n", 
			glist_getcanvas(glist),
			text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),x->image_a,x->image_b,x);
	  
	  
	   sys_vgui("pdsend \"%s _imagesize [image width %lx_imagebang] [image height %lx_imagebang]\"\n", 
                               x->receive->s_name,        x->image_a,                 x->image_a);
	   
	   
     } else {
	  sys_vgui(".x%lx.c coords %lximage %d %d\n",
		   glist_getcanvas(glist), x,
		   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist));
     }

}


void imagebang_erase(t_imagebang* x,t_glist* glist)
{
     int n;
     sys_vgui(".x%lx.c delete %lximage\n",
	      glist_getcanvas(glist), x);

}
	


/* ------------------------ image widgetbehaviour----------------------------- */


static void imagebang_getrect(t_gobj *z, t_glist *glist,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_imagebang* x = (t_imagebang*)z;


    width = x->width;
    height = x->height;
    *xp1 = text_xpix(&x->x_obj, glist);
    *yp1 = text_ypix(&x->x_obj, glist);
    *xp2 = text_xpix(&x->x_obj, glist) + width;
    *yp2 = text_ypix(&x->x_obj, glist) + height;
}

static void imagebang_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_imagebang *x = (t_imagebang *)z;
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    sys_vgui(".x%lx.c coords %lxSEL %d %d %d %d\n",
		   glist_getcanvas(glist), x,
		   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
		   text_xpix(&x->x_obj, glist) + x->width, text_ypix(&x->x_obj, glist) + x->height);

    imagebang_drawme(x, glist, 0);
    canvas_fixlinesfor(glist,(t_text*) x);
}

static void imagebang_select(t_gobj *z, t_glist *glist, int state)
{
    t_imagebang *x = (t_imagebang *)z;
    if (state) {
	    sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxSEL -outline blue\n",
		    glist_getcanvas(glist),
		    text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
		    text_xpix(&x->x_obj, glist) + x->width, text_ypix(&x->x_obj, glist) + x->height,
		    x);
     }
     else {
	     sys_vgui(".x%lx.c delete %lxSEL\n", glist_getcanvas(glist), x);
     }
}


static void imagebang_activate(t_gobj *z, t_glist *glist, int state)
{
/*    t_text *x = (t_text *)z;
    t_rtext *y = glist_findrtext(glist, x);
    if (z->g_pd != gatom_class) rtext_activate(y, state);*/
}

static void imagebang_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    //canvas_deletelinesfor(glist_getcanvas(glist), x);
    canvas_deletelinesfor(glist, x);
}

       
static void imagebang_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_imagebang* s = (t_imagebang*)z;
    if (vis)
	 imagebang_drawme(s, glist, 1);
    else
	 imagebang_erase(s,glist);
}




static void imagebang_size(t_imagebang* x,t_floatarg w,t_floatarg h) {
     x->width = w;
     x->height = h;
}

static void imagebang_flashtime(t_imagebang *x, t_float f)
{
    if (f > 0) 
    {
        x->clockflash = (int)f;
        return;
    }
    t_glist* glist = glist_getcanvas(x->glist);
    if (f == 0)
    {
        sys_vgui(".x%lx.c itemconfigure %lximage -image %lx_imagebang \n", glist, x,x->image_a);
        x->flashing = 0;
        return;
    }
    // f < 0
    sys_vgui(".x%lx.c itemconfigure %lximage -image %lx_imagebang \n", glist, x,x->image_b);
    x->flashing = 1;
}

static void imagebang_imagesize_callback(t_imagebang *x, t_float w, t_float h) {
	DEBUG(post("received w %f h %f",w,h);)
	x->width = w;
	x->height = h;
	canvas_fixlinesfor(x->glist,(t_text*) x);
}
	
	
static void imagebang_free(t_imagebang *x) {
	
	// check first if variable has been unset and image is unused
    // then delete image and unset variable
     DEBUG(sys_vgui("pdsend \"DEBUG b in use [image inuse %lx_imagebang]\"\n",x->image_b);)
     DEBUG(sys_vgui("pdsend \"DEBUG a in use [image inuse %lx_imagebang]\"\n",x->image_a);)
    
    sys_vgui("if { [info exists %lx_imagebang] == 1 && [image inuse %lx_imagebang] == 0} { image delete %lx_imagebang \n unset %lx_imagebang\n} \n",
        x->image_b, x->image_b, x->image_b, x->image_b);
    sys_vgui("if { [info exists %lx_imagebang] == 1 && [image inuse %lx_imagebang] == 0} { image delete %lx_imagebang \n unset %lx_imagebang\n} \n",
        x->image_a, x->image_a, x->image_a, x->image_a);
    
    DEBUG(sys_vgui("pdsend \"DEBUG b exists [info exists %lx_imagebang] \"\n",
        x->image_b);)
    DEBUG(sys_vgui("pdsend \"DEBUG a exists [info exists %lx_imagebang] \"\n",
        x->image_a);)
    
    if (x->receive) {
		pd_unbind(&x->x_obj.ob_pd,x->receive);
	}
	clock_free(x->clock_flash);
	clock_free(x->clock_brk);
}

// Defaults used in case of in images specified, or files not found. Set identical to bang
//static unsigned char *bangOffXbmString = "#define bangOff_width 16\n\
//#define bangOff_height 16\n\
//static unsigned char bangOff_bits[] = {\n\
//   0xff, 0xff, 0xe1, 0x87, 0x31, 0x8c, 0x09, 0x90, 0x05, 0xa0, 0x07, 0xe0,\n\
//   0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x07, 0xe0, 0x05, 0xa0,\n\
//   0x09, 0x90, 0x31, 0x8c, 0xff, 0x87, 0xff, 0xff};\0";
   
//static unsigned char *bangOnXbmString = "#define bangOn_width 16\n\
//#define bangOn_height 16\n\
//static unsigned char bangOn_bits[] = {\n\
//   0xff, 0xff, 0xff, 0x87, 0xf1, 0x8f, 0xf9, 0x9f, 0xfd, 0xbf, 0xff, 0xff,\n\
//   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xbf,\n\
//   0xf9, 0x9f, 0xf1, 0x8f, 0xff, 0x87, 0xff, 0xff };\0";
   
static unsigned char *bangOffXbmString = "#define bangOff_width 16\n\
#define bangOff_height 16\n\
static unsigned char bangOff_bits[] = {\n\
   0xff, 0xff, 0xff, 0xff, 0x3f, 0xfc, 0x0f, 0xf0, 0x07, 0xe0, 0x07, 0xe0,\n\
   0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x07, 0xe0, 0x07, 0xe0,\n\
   0x0f, 0xf0, 0x3f, 0xfc, 0xff, 0xff, 0xff, 0xff };\0";

// all black bangOn from 0.2.0
//static unsigned char *bangOnXbmString = "#define bangOn_width 16\n\
//#define bangOn_height 16\n\
//static unsigned char bangOn_bits[] = {\n\
//   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,\n\
//   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,\n\
//   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };\0";
   
static unsigned char *bangOnXbmString = "#define bangOn_width 16\n\
#define bangOn_height 16\n\
static unsigned char bangOn_bits[] = {\n\
   0xff, 0xff, 0x0f, 0xf0, 0xe7, 0xe7, 0xf3, 0xcf, 0xf9, 0x9f, 0xfd, 0xbf,\n\
   0xfd, 0xbf, 0xfd, 0xbf, 0xfd, 0xbf, 0xfd, 0xbf, 0xfd, 0xbf, 0xf9, 0x9f,\n\
   0xf3, 0xcf, 0xe7, 0xe7, 0x0f, 0xf0, 0xff, 0xff };\0";
   

static unsigned char *bangMaskXbmString = "#define bangMask_width 16\n\
#define bangMask_height 16\n\
static unsigned char bangMask_bits[] = {\n\
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,\n\
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,\n\
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };\0";

   
static void imagebang_createDefaultImage(t_imagebang *x, t_symbol *image, unsigned char *xbmString)
{
        sys_vgui("if { [info exists %lx_imagebang] == 0 } { image create bitmap %lx_imagebang -data \"%s\" -maskdata \"%s\"\n set %lx_imagebang 1\n} \n",
                                     image,                                     image,               xbmString,       bangMaskXbmString, image);
} 

static void imagebang_status(t_imagebang *x) 
{
    post("--==## imagebang %0.1f status ##==--", VERSION);
    post("width:      %d", x->width);
    post("height:     %d", x->height);
    post("image_a:    %s", x->image_a->s_name);
    post("image_b:    %s", x->image_b->s_name);
    post("receive:    %s", (x->receive) ? x->receive->s_name : "-");
    post("send:       %s", (x->send) ? x->send->s_name : "-");
    post("clockflash: %d", x->clockflash);
    post("clockbrk:   %d", x->clockbrk);
    post("flashing;   %d", x->flashing);
    post("compiled against Pd version:  %d.%d.%d",  
        PD_MAJOR_VERSION, PD_MINOR_VERSION, PD_BUGFIX_VERSION);
    int major, minor, bugfix;
    sys_getversion(&major, &minor, &bugfix);
    post("this Pd version: %d.%d.%d", major, minor, bugfix);}

static void imagebang_setsend(t_imagebang *x, t_symbol *newSend)
{
    if (newSend != gensym(""))
    {
        x->send = newSend;
    }
}

static void imagebang_setreceive(t_imagebang *x, t_symbol *newReceive)
{
    if (x->receive) {
		pd_unbind(&x->x_obj.ob_pd,x->receive);
	}
    if (newReceive != gensym(""))
    {
        x->receive = newReceive;
        pd_bind(&x->x_obj.ob_pd, x->receive );
    }
}

static int imagebang_pd_has_logpost() {
    int major, minor, bugfix;
    sys_getversion(&major, &minor, &bugfix);
    if (major > 0 || minor > 42)
        return 1;
    else
        return 0;
}


static void *imagebang_new(t_symbol *s, int argc, t_atom *argv)
{
    t_imagebang *x = (t_imagebang *)pd_new(imagebang_class);

    x->glist = (t_glist*) canvas_getcurrent();
    
    // Set up a callback to get the size
    x->width = 10;
    x->height = 10;
	
	x->flashing   = 0;
    x->clockflash = CLOCKFLASH;
    x->clockbrk   = CLOCKBRK;

	
	x->image_a = NULL;
	x->image_b = NULL;
	
	t_symbol* image_a = NULL;
	t_symbol* image_b = NULL;
	
	const char *fname;
    
	// CREATE IMAGES
	// images are only created if they have not been created yet
	// we use the symbol pointer to distinguish between image files
	
	if ( argc && (argv)->a_type == A_SYMBOL ) {
		image_a = atom_getsymbol(argv);
		fname = imagebang_get_filename(x, image_a->s_name); // Get image file path
		if (fname) {
			x->image_a = gensym(fname);
			//sys_vgui("set %x_a \"%s\" \n",x,fname);
			// Create the image only if the class has not already loaded the same image (with the same symbolic path name)
			sys_vgui("if { [info exists %lx_imagebang] == 0 } { image create photo %lx_imagebang -file \"%s\"\n set %lx_imagebang 1\n} \n",
                                                     x->image_a,                               x->image_a,          fname,     x->image_a); 
//            sys_vgui("pdsend {test %x_imagebang}\n", x->image_a);
		} else {
            if (imagebang_pd_has_logpost())
                logpost(NULL, 4, "[imagebang] could not find \"%s\" for off-image. Will use default.", image_a->s_name);
		}
    }

    if ( argc > 1 && (argv+1)->a_type == A_SYMBOL ) {
		image_b = atom_getsymbol(argv+1);
		fname = imagebang_get_filename(x, image_b->s_name); // Get image file path
		if (fname) {
			x->image_b = gensym(fname);
			//sys_vgui("set %x_b \"%s\" \n",x,fname);
			sys_vgui("if { [info exists %lx_imagebang] == 0} { image create photo %lx_imagebang -file \"%s\"\n set %lx_imagebang 1\n} \n",
                                                     x->image_b,                              x->image_b,          fname,     x->image_b);
//            sys_vgui("pdsend {test %x_imagebang}\n", x->image_b);
		} else {
            if (imagebang_pd_has_logpost())
                logpost(NULL, 4, "[imagebang] could not find \"%s\" for on-image. Will use default.", image_b->s_name);
        }
    }

	// Plan B; use build in defaults (which look similar but different to a bang)
	if (x->image_a == NULL) {
        image_a = gensym("bangOff");
        imagebang_createDefaultImage(x, image_a, bangOffXbmString);
        x->image_a = image_a;
    }        
    if (x->image_b == NULL) {
        image_b = gensym("bangOn");
        imagebang_createDefaultImage(x, image_b, bangOnXbmString);
        x->image_b = image_b;
    }

	x->send = NULL;
	if ( argc > 2 && (argv+2)->a_type == A_SYMBOL ) {
		x->send = atom_getsymbol(argv+2);
	}
	
	if ( argc > 3 && (argv+3)->a_type == A_SYMBOL ) {
		x->receive = atom_getsymbol(argv+3);
	} else {
	   // Create default receiver if none set
		char buf[MAXPDSTRING];
		sprintf(buf, "#%lx", (long)x);
		x->receive = gensym(buf);
	}
	
    pd_bind(&x->x_obj.ob_pd, x->receive );
    
    x->clock_flash = clock_new(x, (t_method)imagebang_flash_timeout);
    x->clock_brk = clock_new(x, (t_method)imagebang_brk_timeout);
    
    
   x->outlet = outlet_new(&x->x_obj, &s_float);
   

   return (x);
   
}

void imagebang_setup(void)
{
    imagebang_class = class_new(gensym("imagebang"), (t_newmethod)imagebang_new, (t_method)imagebang_free,
				sizeof(t_imagebang),0, A_GIMME,0);

    class_addmethod(imagebang_class, (t_method)imagebang_imagesize_callback,\
                    gensym("_imagesize"), A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(imagebang_class, (t_method)imagebang_status,\
                    gensym("status"), 0);
    class_addmethod(imagebang_class, (t_method)imagebang_setsend,\
                    gensym("send"), A_DEFSYMBOL, 0);
    class_addmethod(imagebang_class, (t_method)imagebang_setreceive,\
                    gensym("receive"), A_DEFSYMBOL, 0);
    class_addmethod(imagebang_class, (t_method)imagebang_flashtime,\
                    gensym("flashtime"), A_DEFFLOAT, 0);

    class_addbang(imagebang_class,     (t_method)imagebang_bang);
    class_addfloat(imagebang_class,    (t_method)imagebang_bang);
    class_addlist(imagebang_class,     (t_method)imagebang_bang);
    class_addsymbol(imagebang_class,   (t_method)imagebang_bang);
    class_addanything(imagebang_class, (t_method)imagebang_bang);
    
    imagebang_widgetbehavior.w_getrectfn =  imagebang_getrect;
    imagebang_widgetbehavior.w_displacefn = imagebang_displace;
    imagebang_widgetbehavior.w_selectfn =   imagebang_select;
    imagebang_widgetbehavior.w_activatefn = imagebang_activate;
    imagebang_widgetbehavior.w_deletefn =   imagebang_delete;
    imagebang_widgetbehavior.w_visfn =      imagebang_vis;

    imagebang_widgetbehavior.w_clickfn = (t_clickfn)imagebang_click;
    
#if PD_MINOR_VERSION < 37
	imagebang_widgetbehavior.w_propertiesfn = NULL; 
    //imagebang_widgetbehavior.w_savefn =   imagebang_save;
#endif

    
    class_setwidget(imagebang_class,&imagebang_widgetbehavior);
#if PD_MINOR_VERSION >= 37
   // class_setsavefn(imagebang_class,&imagebang_save);
#endif

    if (imagebang_pd_has_logpost())
        logpost(NULL, 4, "tof/imagebang version %1.1f", VERSION);
}


