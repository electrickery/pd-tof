/* w_breakpoints~ header for PD by Thomas Ouellet Fredericks           *
 * Based on [ggee/envgen] by Guenter Geiger                            *
 * 
 * This code is licensed under the 3-clause BSD license                *
 * fjkraan@xs4all.nl. 2016-06-14                                       *
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef _WIN32
//#pragma warning( disable : 4244 )
//#pragma warning( disable : 4305 )
#define abs fabs
#endif

#define BACKGROUNDCOLOR "grey86"
#define LINECOLOR "grey30"

#define BORDER 2
// IOWIDTH comes from g_canvas.h
#define IOHEIGHT 1
// $canvas create rectangle $x-pos $y-pos $width $height [--tags ...]
//#define DEBUG 1

static void draw_iolets(t_breakpoints *x, t_glist *glist, int firsttime, int nin, int nout)
{

    if (x->r_sym == &s_) 
    {
        int n = nout;
        int nplus, i;
        int wxpos = text_xpix(&x->x_obj,glist);
        int wypos = text_ypix(&x->x_obj,glist);
        int zoom = x->x_zoom;

        nplus = (n == 1 ? 1 : n-1);
        for (i = 0; i < n; i++)
        {
            int x_onset = wxpos + ((x->w.width - IOWIDTH + 3 * BORDER) * i / nplus - BORDER) * zoom;
            int y_onset = wypos + (x->w.height - 1 + 2 * BORDER) * zoom;
            // OUTLETS
            if (firsttime)
            {
                sys_vgui(".x%lx.c create rectangle %d %d %d %d -width %d -tags %lxo%d \n",
                    glist_getcanvas(glist),
                    x_onset,                  y_onset,
                    x_onset + IOWIDTH * zoom, y_onset - IOHEIGHT * zoom,
                    1 * zoom, x, i);
            }
            else
            {
                sys_vgui(".x%lx.c coords %lxo%d %d %d %d %d\n",
                    glist_getcanvas(glist), x, i,
                    x_onset,                  y_onset,
                    x_onset + IOWIDTH * zoom, y_onset - IOHEIGHT * zoom);
            }
        }
        n = nin; 
        nplus = (n == 1 ? 1 : n - 1);
        for (i = 0; i < n; i++)
        {
            int x_onset = wxpos + ((x->w.width - IOWIDTH + 3 * BORDER) * i / nplus - BORDER) * zoom;
            int y_onset = wypos - BORDER;
            // INLETS
            if (firsttime)
            {
                sys_vgui(".x%lx.c create rectangle %d %d %d %d -width %d -tags %lxi%d\n",
                    glist_getcanvas(glist),
                    x_onset,                  y_onset,
                    x_onset + IOWIDTH * zoom, y_onset + IOHEIGHT * zoom,
                    1 * zoom, x, i);
            }
            else
            {
                sys_vgui(".x%lx.c coords %lxi%d %d %d %d %d\n",
                    glist_getcanvas(glist), x, i,
                    x_onset,                  y_onset,
                    x_onset + IOWIDTH * zoom, y_onset + IOHEIGHT * zoom);
             }
         }
    }
}

static int breakpoints_next_doodle(t_breakpoints *x, struct _glist *glist,
                              int xpos, int ypos)
{
    float xscale,yscale;
    int dxpos,dypos;
    float minval = 100000.0;
    float tval;
    int i;
    int insertpos = -1;
    float ySize = x->max - x->min;
    float yBase =  x->min;
    int zoom = x->x_zoom;
    int wxpos = text_xpix(&x->x_obj, glist);
    int wypos = text_ypix(&x->x_obj, glist);
    if (xpos > wxpos + x->w.width) 
        xpos = wxpos + x->w.width;

    xscale = x->w.width / x->duration[x->last_state];
    yscale = x->w.height;
     
    dxpos = wxpos / zoom;/* + BORDER */;
    dypos = wypos / zoom + BORDER;

    for (i = 0; i <= x->last_state; i++) {
        float dx2 = (dxpos + (x->duration[i] * xscale)) - xpos;
        float dy2 = (dypos + yscale - ( (x->finalvalues[i] - yBase) / ySize * yscale)) - ypos;

        dx2 *= dx2;
        dy2 *= dy2;
        tval = sqrt(dx2 + dy2);

        if (tval <= minval) {
            minval = tval;	    
            insertpos = i;
        }
    }

     /* decide if we want to make a new one */
    if (minval > /*5*/ 8 && insertpos >= 0 && !x->x_freeze) {

        while (((dxpos + (x->duration[insertpos] * xscale)) - xpos) < 0)
            insertpos++;
        while (((dxpos + (x->duration[insertpos - 1] * xscale)) - xpos) > 0)
            insertpos--;

        if (x->last_state+1 >= x->args)
            breakpoints_resize(x,x->args+1);

        for (i = x->last_state; i >= insertpos; i--) {
            x->duration[i + 1]    = x->duration[i];
            x->finalvalues[i + 1] = x->finalvalues[i];
        }
        x->duration[insertpos] = (float)(xpos - dxpos) / x->w.width * x->duration[x->last_state++];
        x->w.pointerx = xpos;
        x->w.pointery = ypos;
    }
    else {
        x->w.pointerx = dxpos + x->duration[insertpos] * x->w.width / x->duration[x->last_state]; 

        x->w.pointery = ypos;
        //x->w.pointery = wypos +  (1.f - (x->finalvalues[i] - yBase) / ySize) * yscale;
    }
#ifdef DEBUG
    post("pointery =%f", x->w.pointery);
    post("insertpos =%f", insertpos);
#endif
    x->w.grabbed = insertpos;
    return insertpos;
}

static void breakpoints_create_doodles(t_breakpoints *x, t_glist *glist)
{
    float xscale, yscale;
    int xpos, ypos;
    int i;
    char guistr[255];
    float ySize = x->max - x->min;
    float yBase =  x->min;
    float yvalue;
    int zoom = x->x_zoom;
    
    xscale = x->w.width/x->duration[x->last_state];
    yscale = x->w.height;
    int doodleRadius = 2;
     
    int wxpos = text_xpix(&x->x_obj,glist);
    int wypos = text_ypix(&x->x_obj,glist);
    for (i = 0; i <= x->last_state; i++) {
        yvalue = ((x->finalvalues[i] - yBase) / ySize * yscale) * zoom;
        sprintf(guistr,".x%lx.c create oval %d %d %d %d -tags %lxD%d", glist_getcanvas(glist),
            (int) (wxpos + x->duration[i] * xscale * zoom - doodleRadius * zoom),
            (int) (wypos + x->w.height    * zoom - yvalue - doodleRadius * zoom),
            (int) (wxpos + x->duration[i] * xscale * zoom + doodleRadius * zoom),
            (int) (wypos + x->w.height    * zoom - yvalue + doodleRadius * zoom),
            x,i);

        if (i == x->w.grabbed) {
            strcat(guistr," -fill red\n");
        } else {
            strcat(guistr," -fill "LINECOLOR"\n");
        }
        sys_vgui("%s", guistr);
    }
    x->w.numdoodles = i;
}


static void breakpoints_delete_doodles(t_breakpoints *x, t_glist *glist)
{
     int i;
     for (i=0;i<=x->w.numdoodles;i++) {
         sys_vgui(".x%lx.c delete %lxD%d\n", glist_getcanvas(glist), x, i);
     }
}

static void breakpoints_update_doodles(t_breakpoints *x, t_glist *glist)
{
     breakpoints_delete_doodles(x, glist);
/* LATER only create new doodles if necessary */
     breakpoints_create_doodles(x, glist);
}


static void breakpoints_delnum(t_breakpoints *x)
{
     sys_vgui(".x%lx.c delete %lxT\n", glist_getcanvas(x->w.glist), x); 
}


static void breakpoints_shownum(t_breakpoints *x,t_glist* glist) 
{
    int i = x->w.grabbed;
    float ySize = x->max - x->min;
    float yBase =  x->min;
    int zoom = x->x_zoom;
    
    float xscale = x->w.width/x->duration[x->last_state];
    float yscale = x->w.height / ySize;
     
    int wxpos = text_xpix(&x->x_obj,glist);
    int wypos = text_ypix(&x->x_obj,glist);

    breakpoints_delnum(x);

    sys_vgui(".x%lx.c create text %d %d -text %.2fx%.2f -tags %lxT\n",
        glist_getcanvas(x->w.glist),
        (int) (wxpos + ((x->duration[i] * xscale) + 3) * zoom),
        (int) (wypos + x->w.height - ((x->finalvalues[i] - yBase) * yscale - 10) * zoom),
        x->duration[i],
        x->finalvalues[i],
        x);
    clock_delay(x->w.numclock, 700);
}



static void breakpoints_create_background(t_breakpoints *x, t_glist *glist)
{
    int wxpos = text_xpix(&x->x_obj,glist);
    int wypos = text_ypix(&x->x_obj,glist);
    
    int zoom = x->x_zoom;
    int zWidth  = x->w.width * zoom;
    int zHeight = x->w.height * zoom;
    int zBorder = BORDER * zoom;

    x->w.numclock = clock_new(x, (t_method) breakpoints_delnum);     
    sys_vgui(".x%lx.c create rectangle %d %d %d %d  -tags %lxS -fill %s -width %d\n",
        glist_getcanvas(glist),
        wxpos - zBorder, wypos - zBorder,
        wxpos + zWidth + 2 * zBorder, wypos + zHeight + 2 * zBorder,
        x, BACKGROUNDCOLOR, x->borderwidth * zoom);
}

static void breakpoints_create_lines(t_breakpoints *x, t_glist *glist)
{
    int i;
    float ySize = x->max - x->min;
    float yBase =  x->min;
    float xscale = x->w.width / x->duration[x->last_state];
    float yscale = x->w.height;
    int zoom = x->x_zoom;
     
    int wxpos = text_xpix(&x->x_obj,glist);
    int wypos = text_ypix(&x->x_obj,glist);
    sys_vgui(".x%lx.c create line", glist_getcanvas(glist));
    for (i = 0; i <= x->last_state; i++) {
         sys_vgui(" %d %d ",
             (int)(wxpos + x->duration[i] * xscale * zoom),
             (int)(wypos + x->w.height * zoom - ((x->finalvalues[i] - yBase) / ySize * yscale) * zoom));
    }
    sys_vgui(" -tags %lxP -fill %s\n", x, LINECOLOR);
}
     
static void breakpoints_create(t_breakpoints *x, t_glist *glist)
{
    breakpoints_create_background(x, glist);
    breakpoints_create_lines(x, glist);
    breakpoints_create_doodles(x, glist);
}


static void breakpoints_update_background(t_breakpoints *x, t_glist *glist)
{
    int wxpos = text_xpix(&x->x_obj,glist);
    int wypos = text_ypix(&x->x_obj,glist);
    int zoom = x->x_zoom;
    int zWidth = (x->w.width + 2 * BORDER) * zoom;
    int zHeight = (x->w.height + 2 * BORDER) * zoom;
    int zBorder = BORDER * zoom;
    
    sys_vgui(".x%lx.c coords %lxS %d %d %d %d\n",
        glist_getcanvas(glist), x,
        wxpos - zBorder, wypos - zBorder,
        wxpos + zWidth,  wypos + zHeight);
}

static void breakpoints_update_lines(t_breakpoints *x, t_glist *glist)
{     
    float xscale, yscale;
    int wxpos = text_xpix(&x->x_obj, glist);
    int wypos = text_ypix(&x->x_obj, glist);
    int zoom = x->x_zoom;
    float ySize = x->max - x->min;
    float yBase =  x->min;

     xscale = x->w.width / x->duration[x->last_state];
     yscale = x->w.height;
     
     sys_vgui(".x%lx.c coords %lxP", glist_getcanvas(glist), x);
     int i;
     for (i = 0; i <= x->last_state; i++) {
         
         sys_vgui(" %d %d ",
             (int)(wxpos + x->duration[i] * xscale * zoom),
             (int)(wypos + x->w.height * zoom - ((x->finalvalues[i] - yBase) / ySize * yscale) * zoom));
     }
     sys_vgui("\n"); 
}
 
static void breakpoints_update(t_breakpoints *x, t_glist *glist)
{
    breakpoints_update_background(x,glist);
    breakpoints_update_lines(x,glist);     
    breakpoints_update_doodles(x,glist);
    draw_iolets(x, glist, 0, 1, 3);
}



static void breakpoints_drawme(t_breakpoints *x, t_glist *glist, int firsttime)
{
    if (firsttime) 
        breakpoints_create(x,glist);
    else 
        breakpoints_update(x,glist);

    draw_iolets(x, glist, firsttime, 1,3);
}




static void breakpoints_erase(t_breakpoints* x, t_glist* glist)
{
    // packground
    sys_vgui(".x%lx.c delete %lxS\n",
        glist_getcanvas(glist), x);

    //lines
    sys_vgui(".x%lx.c delete %lxP\n",
        glist_getcanvas(glist), x);

    // in- & out-lets
    if (x->r_sym == &s_) {
        sys_vgui(".x%lx.c delete %lxi0\n", glist_getcanvas(glist), x);
        sys_vgui(".x%lx.c delete %lxo0\n", glist_getcanvas(glist), x);
        sys_vgui(".x%lx.c delete %lxo1\n", glist_getcanvas(glist), x);
        sys_vgui(".x%lx.c delete %lxo2\n", glist_getcanvas(glist), x);
    }
    breakpoints_delete_doodles(x, glist);
}



/* ------------------------ breakpoints widgetbehaviour----------------------------- */


static void breakpoints_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_breakpoints* x = (t_breakpoints*)z;
    int zoom = x->x_zoom;

    int width  = x->w.width  + 2 * BORDER;
    int height = x->w.height + 2 * BORDER;
    *xp1 = text_xpix(&x->x_obj, owner) - BORDER * zoom;
    *yp1 = text_ypix(&x->x_obj, owner) - BORDER * zoom;
    *xp2 = text_xpix(&x->x_obj, owner) + width * zoom; //+ 4
    *yp2 = text_ypix(&x->x_obj, owner) + height *zoom; //+ 4
}

static void breakpoints_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_breakpoints *x = (t_breakpoints *)z;
    x->x_obj.te_xpix += dx; // wxpos
    x->x_obj.te_ypix += dy; // wypos

    breakpoints_drawme(x, glist, 0);
    canvas_fixlinesfor(glist,(t_text*) x);
}

static void breakpoints_select(t_gobj *z, t_glist *glist, int state)
{
    // background
    t_breakpoints *x = (t_breakpoints *)z;
    sys_vgui(".x%lx.c itemconfigure %lxS -fill %s\n", glist, 
        x, (state? "blue" : BACKGROUNDCOLOR));
}

static void breakpoints_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist, x);
}

       
static void breakpoints_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_breakpoints* x = (t_breakpoints*)z;
    if (vis)
        breakpoints_drawme(x, glist, 1);
    else
        breakpoints_erase(x, glist);
}

static void breakpoints_followpointer(t_breakpoints* x,t_glist* glist)
{
    float dur;
    float xscale = x->duration[x->last_state] / x->w.width;
    float ySize = x->max - x->min;
    float yBase =  x->min;
    int wxpos = text_xpix(&x->x_obj,glist);
    int wypos = text_ypix(&x->x_obj,glist);
    int zoom = x->x_zoom;

    if  ((x->w.grabbed > 0) && (x->w.grabbed < x->last_state)) {
         dur = (x->w.pointerx - wxpos / zoom) * xscale;
        if (dur < x->duration[x->w.grabbed - 1])
            dur = x->duration[x->w.grabbed - 1];
        if (dur > x->duration[x->w.grabbed + 1])
            dur = x->duration[x->w.grabbed + 1];

        x->duration[x->w.grabbed] = dur;
    }
     
    float grabbed = (1.0f - (x->w.pointery - (float)wypos / zoom) / (float)x->w.height);
#ifdef DEBUG
    post("grabbed =%f",grabbed);
#endif
     
    if (grabbed < 0.0) 
        grabbed = 0.0;
    else if (grabbed > 1.0)
        grabbed = 1.0;
  
    x->finalvalues[x->w.grabbed] = grabbed * ySize + yBase;

    outlet_bang(x->out3);
    if (x->c_sym != &s_) 
        pd_bang(x->c_sym->s_thing);
}


static void breakpoints_motion(t_breakpoints *x, t_floatarg zdx, t_floatarg zdy)
{
    int zoom = x->x_zoom;
    float dx = zdx / zoom;
    float dy = zdy / zoom;
    if (x->w.shift) 
    {
        x->w.pointerx += dx / 1000.f;
        x->w.pointery += dy / 1000.f;
    }
    else
    {
        x->w.pointerx += dx;
        x->w.pointery += dy;
    }
    if (!x->resizing)
        breakpoints_followpointer(x, x->w.glist);
    else 
    {
        x->w.width  += dx;
        x->w.height += dy;
    }
    breakpoints_shownum(x, x->w.glist);
    breakpoints_update(x, x->w.glist);
}

static void breakpoints_key(t_breakpoints *x, t_floatarg f)
{
    if (f == 8.0 && x->w.grabbed < x->last_state &&  x->w.grabbed > 0) 
    {
        int i;
        for (i = x->w.grabbed; i <= x->last_state; i++) 
        {
             x->duration[i]    = x->duration[i + 1];
             x->finalvalues[i] = x->finalvalues[i + 1];
        }

        x->last_state--;
        x->w.grabbed--;
        breakpoints_update(x, x->w.glist);
        outlet_bang(x->out3);
        if (x->c_sym != &s_) 
            pd_bang(x->c_sym->s_thing);
     }
}

static int breakpoints_newclick(t_breakpoints *x, struct _glist *glist,
    int zxpos, int zypos, int shift, int alt, int dbl, int doit)
{
    // check if user wants to resize 
    float wxpos = text_xpix(&x->x_obj,glist);
    float wypos = (text_ypix(&x->x_obj,glist));
    int zoom = x->x_zoom;
    int xpos = zxpos / zoom;
    int ypos = zypos / zoom;
    
    if (doit) 
    {
     
#ifdef DEBUG
     post("clicked");
#endif

        if ((zxpos >= wxpos ) && (zxpos <= wxpos + x->w.width * zoom ) \
            && (zypos >= wypos ) && (zypos <= wypos + x->w.height * zoom ) ) {

#ifdef DEBUG
            post("inside");
#endif

            breakpoints_next_doodle(x, glist, xpos, ypos);

            glist_grab(x->w.glist, &x->x_obj.te_g, 
                (t_glistmotionfn) breakpoints_motion,
                (t_glistkeyfn) breakpoints_key, zxpos, zypos);
             
            x->w.shift = shift;
            breakpoints_followpointer(x, glist);
            breakpoints_shownum(x, glist);
            breakpoints_update(x, glist);
        }
    }
    return (1);
}

void breakpoints_zoom(t_breakpoints *x, t_floatarg zoom)
{
     x->x_zoom = (zoom < 1) ? 1 : (int)zoom;
}
