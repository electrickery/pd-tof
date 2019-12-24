
#define UPDATE 0
#define CREATE 1
#define DESTROY 2

static int menubutton_w_is_visible(t_menubutton* x) {
	
	return (x->x_glist)?glist_isvisible(x->x_glist):0;
	
}

/* CHANGES
 * 
 * 0.3.2 cleanup and added zoom functionality (fjk, 2019-12-24).
 * 0.3.1 changed %x to %lx. This failed on MaxOSX-64-bit (fjk, 2016-10-30).
 * 0.3   first versioned version (derived from help-patch).
 * 
 */


static void menubutton_w_disable(t_menubutton*x, t_float f) {
 
  int i = (int)f;
    sys_vgui(".x%lx.c.s%lx configure -state \"%s\"\n", 
        x->x_glist, x, i?"disabled":"active");
}

static void menubutton_w_text(t_menubutton* x, t_symbol* s) {
    sys_vgui(".x%lx.c.s%lx configure -text \"%s\"\n",
        x->x_glist, x, s->s_name);
}

static void menubutton_w_clear(t_menubutton* x) {
    sys_vgui(".x%lx.c.s%lx.menu delete 0 end ; .x%lx.c.s%lx configure -text \"%s\"\n", 
        x->x_glist, x,x->x_glist, x,"");
}

static void menubutton_w_additem(t_menubutton* x, t_symbol *label, int i) {
    sys_vgui(".x%lx.c.s%lx.menu add command -label \"%s\" \
        -command {.x%lx.c.s%lx configure -text \"%s\" ; select%lx \"%d\"} \n",
        x->x_glist, x, label->s_name, 
        x->x_glist, x, label->s_name, x, i);

}

//-anchor %s 

static void menubutton_w_set_align(t_menubutton* x) {
    if ( x->halign == -1 ) {
        sys_vgui(".x%lx.c.s%lx configure -anchor w\n", x->x_glist, x);
    } else if ( x->halign == 0 ) {
        sys_vgui(".x%lx.c.s%lx configure -anchor center\n", x->x_glist, x);
    } else {
        sys_vgui(".x%lx.c.s%lx configure -anchor e\n", x->x_glist, x);
    }
}

static void menubutton_w_apply_colors(t_menubutton* x) {

    sys_vgui(".x%lx.c.s%lx configure -background \"%s\" -foreground \"%s\" -activeforeground \"%s\" -activebackground \"%s\"\n", 
        x->x_glist, x, x->bg_color->s_name, x->fg_color->s_name, x->fg_color->s_name, x->hi_color->s_name);

    sys_vgui(".x%lx.c.s%lx.menu configure -background \"%s\" -foreground \"%s\" -activeforeground \"%s\" -activebackground \"%s\"\n", 
        x->x_glist, x, x->bg_color->s_name, x->fg_color->s_name, x->fg_color->s_name, x->hi_color->s_name);

    sys_vgui(".x%lx.c itemconfigure  %lxR -outline \"%s\"\n", 
        x->x_glist, x, x->co_color->s_name);
}

static void menubutton_w_draw_iolets(t_menubutton *x, t_glist *glist, int draw, int nin, int nout)
{
    int zoom = x->x_zoom;
    int wxpos = text_xpix(&x->x_obj, glist);
    int wypos = text_ypix(&x->x_obj, glist);
    int zWidth  = x->x_width  * zoom;
    int zHeight = x->x_height * zoom;
    int zIOWidth = IOWIDTH * zoom;

 /* outlets */
     int n = nin;
     int nplus, i;
     nplus = (n == 1 ? 1 : n-1);
     DEBUG(post("draw iolet");)
     for (i = 0; i < n; i++)
     {
        int onset = wxpos + zWidth - zIOWidth) * i / nplus;
        if (draw == CREATE)
        {
            sys_vgui(".x%lx.c create rectangle %d %d %d %d -outline blue -tags {%lxo%d %lxo}\n",
                glist_getcanvas(glist),
                onset,            wypos + zHeight + 1 * zoom,
                onset + zIOWidth, wypos, glist) + zHeight + 2 * zoom,
                x, i, x);
        } 
        else if (draw == UPDATE)
        {
            sys_vgui(".x%lx.c coords %lxo%d %d %d %d %d\n",
                glist_getcanvas(glist), x, i,
                onset,            wypos + zHeight + 1 * zoom,
                onset + zIOWidth, wypos + zHeight + 2 * zoom);
        }
        else
        { // DELETE
            sys_vgui(".x%lx.c delete %lxo\n", 
                glist_getcanvas(glist), x); // Added tag for all outlets of one instance 
        }
    }
 /* inlets */
    n = nout; 
    nplus = (n == 1 ? 1 : n-1);
    for (i = 0; i < n; i++)
    {
        int onset = wxpos + (zWidth - zIOWidth) * i / nplus;
        if (draw==CREATE) {
            sys_vgui(".x%lx.c create rectangle %d %d %d %d -outline blue -tags {%lxi%d %lxi}\n",
                glist_getcanvas(glist),
                onset,            wypos - 2 * zoom,
                onset + zIOWidth, wypos - 1 * zoom,
                x, i, x);
        } else if (draw==UPDATE) {
            sys_vgui(".x%lx.c coords %lxi%d %d %d %d %d\n",
                glist_getcanvas(glist), x, i,
                onset,            wypos - 2 * zoom,
                onset + zIOWidth, wypos - 1 * zoom);
        } else { // DELETE
        sys_vgui(".x%lx.c delete %lxi\n", 
            glist_getcanvas(glist), x); // Added tag for all inlets of one instance
        }
    }
    DEBUG(post("draw iolet end");)
}


static void menubutton_w_draw_handle(t_menubutton *x, t_glist *glist, int draw)
{
    DEBUG(post("draw_handle start: %i",draw);)

    int zoom = x->x_zoom;
    int wxpos = text_xpix(&x->x_obj, glist);
    int wypos = text_ypix(&x->x_obj, glist);
    int zWidth  = x->x_width  * zoom;
    int zHeight = x->x_height * zoom;

    int onset = wxpos;
  
    if (draw == CREATE)
    {
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxhandle -outline blue -fill blue\n",
            glist_getcanvas(glist),
            onset,          wypos,
            onset + zWidth, wypos + zHeight ,
            x);
    }
    else if (draw == UPDATE)
    {
        sys_vgui(".x%lx.c coords %lxhandle %d %d %d %d\n",
            glist_getcanvas(glist), x, 
            onset,          wypos,
            onset + zWidth, wypos + zHeight);
    }
    else // DELETE
    {
        sys_vgui(".x%lx.c delete  %lxhandle\n", glist_getcanvas(glist), x, 0);
    }
    DEBUG(post("draw_handle end");)
}

static void menubutton_w_create_widget(t_menubutton *x)
{
    DEBUG(post("create_widget start");)

    char text[MAXPDSTRING];
    int len,i;
    t_symbol* temp_name;
  
    /* Create menubutton and empty menu widget -- maybe the menu should be created elseware?*/

    /* draw using the last name if it was selected otherwise use default name. */
    if (x->current_selection < 0)
    {
        temp_name = gensym("");
    }
    else
    {
        temp_name = x->x_options[x->current_selection];
    }

  /* Seems we have to delete the widget in case it already exists (Provided by Guenter)*/
  
    if (x->initialized)
    {
        DEBUG(post("destroying previous widget");)
        sys_vgui("destroy .x%lx.c.s%lx\n", x->x_glist, x);
      
        // Create menubutton and menu 
      
        sys_vgui("set %lxw .x%lx.c.s%lx ; menubutton $%lxw \
            -justify left -relief flat -indicatoron 0 -text \"%s\" \
            -direction flush -menu $%lxw.menu ; menu $%lxw.menu -relief solid \
            -tearoff 0 \n",
            x, x->x_glist, x, x, temp_name->s_name, x, x);

        menubutton_w_set_align(x);

        menubutton_w_apply_colors(x);
       
        // Add a binding 
        //post("Trying to bind");
        //sys_vgui("bind Menubutton <ButtonPress-1> {} \n", x);
        //sys_vgui("bind $%xw <ButtonPress-1> { } \n", x);
       
        for(i = 0; i < x->x_num_options; i++)
        {
            // Add menu itmes
            menubutton_w_additem(x, x->x_options[i], i);
        }
    }

    DEBUG(post("id: .x%lx.c.s%lx", x->x_glist, x);)
    DEBUG(post("create_widget end");)
}

static void menubutton_w_draw_contour(t_menubutton *x, t_glist *glist, int draw) {
    int zoom = x->x_zoom;
    int wxpos = text_xpix(&x->x_obj, glist);
    int wypos = text_ypix(&x->x_obj, glist);
    int zWidth  = x->x_width  * zoom;
    int zHeight = x->x_height * zoom;
    
    if (draw == CREATE) {
        sys_vgui(".x%lx.c create rectangle %d %d %d %d -tags %lxR -outline \"%s\" \n",
            glist_getcanvas(glist),
            wxpos, wypos,
            wxpos + zWidth, wypos + zHeight,
            x, x->co_color->s_name); 
    } else if (draw == UPDATE) {
        sys_vgui(".x%lx.c coords %lxR %d %d %d %d\n",
            glist_getcanvas(glist), x, 
            wxpos, wypos,
            wxpos + zWidth, wypos + zHeight );
    } else { // DELETE
       sys_vgui(".x%lx.c delete  %lxR\n", glist_getcanvas(glist),x,0);
    }
}

static void  menubutton_w_resize(t_menubutton* x) {
    int zoom = x->x_zoom;
    int zWidth  = x->x_width  * zoom;
    int zHeight = x->x_height * zoom;
    
    sys_vgui(".x%lx.c itemconfigure %lxS -width %i -height %i \n", 
        x->x_glist, x, zWidth - 1, zHeight - 1);
    menubutton_w_draw_contour(x, x->x_glist, UPDATE);
    canvas_fixlinesfor(x->x_glist, (t_text*) x);
}

static void menubutton_w_drawme(t_menubutton *x, t_glist *glist, int draw)
{
    int zoom = x->x_zoom;
    int wxpos = text_xpix(&x->x_obj, glist);
    int wypos = text_ypix(&x->x_obj, glist);
    int zWidth  = x->x_width  * zoom;
    int zHeight = x->x_height * zoom;
    int offset = 1 * zoom;
    
    t_canvas *canvas = glist_getcanvas(glist);
    DEBUG(post("drawme start");)
    DEBUG(post("drawme %d", draw);)
    if (draw == CREATE)
    {
        /* by drawing, we "initialize" the popup */
        x->initialized = 1;

        DEBUG(post("glist %lx canvas %lx", x->x_glist, canvas);)
        menubutton_w_draw_contour(x, glist, CREATE);
        x->x_glist = canvas;
        menubutton_w_create_widget(x);       
       
        sys_vgui(".x%lx.c create window %d %d -width %d -height %d -anchor nw -window .x%lx.c.s%lx -tags %lxS\n", 
            canvas, wxpos + offset, wypos + offset, zWidth - offset, zHeight - offset, x->x_glist, x, x);
    }     
    else if (draw == UPDATE)
    {
        menubutton_w_draw_contour(x, glist, UPDATE);
        sys_vgui(".x%lx.c coords %lxS %d %d\n",
        canvas, x,
        wxpos + offset, wypos + offset);
    } 
    else
    { // DELETE
        DEBUG(post("erase start");)
        if (x->initialized) {
            menubutton_w_draw_contour(x, glist, DESTROY);	
            sys_vgui("destroy .x%lx.c.s%lx\n", glist_getcanvas(glist), x);
            sys_vgui(".x%lx.c delete %lxS\n", glist_getcanvas(glist), x);
        }
        DEBUG(post("erase end");)
    }

    // menubutton_w_draw_handle(x, glist, firsttime);
    //sys_vgui(".x%x.c.s%x configure -state \"%s\"\n", canvas, x, x->x_disabled?"disabled":"active");

    // Output a bang to first outlet when we're ready to receive float messages the first time!. 
    // Too bad this is NOT always the first time... window shading makes the bang go out again. :(
    //if(firsttime) {outlet_bang(x->x_obj.ob_outlet);}

    DEBUG(post("drawme end");)
}



/* ------------------------ popup widgetbehaviour----------------------------- */


static void menubutton_w_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    // DEBUG(post("getrect start");)

    t_menubutton* x = (t_menubutton*)z;
    int zoom = x->x_zoom;

    int width = x->x_width * zoom;
    int height = x->x_height * zoom;
    *xp1 = text_xpix(&x->x_obj, owner) ;
    *yp1 = text_ypix(&x->x_obj, owner) ;
    *xp2 = text_xpix(&x->x_obj, owner) + width ;
    *yp2 = text_ypix(&x->x_obj, owner) + height ;
  
    // DEBUG(post("getrect end");)
}

static void menubutton_w_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_menubutton *x = (t_menubutton *)z;
    DEBUG(post("displace start");)
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if (glist_isvisible(glist))
    {
        menubutton_w_draw_handle(x, glist, UPDATE);
        menubutton_w_draw_inlets(x, glist, UPDATE, 1, 1);
        menubutton_w_drawme(x, glist, UPDATE);
        canvas_fixlinesfor(glist,(t_text*) x);
    }
    DEBUG(post("displace end");)
}

static void menubutton_w_select(t_gobj *z, t_glist *glist, int state)
{
    DEBUG(post("select start");)

    t_menubutton *x = (t_menubutton *)z;
    if (x->initialized) {

        if (state)
        {
            menubutton_w_draw_handle(x, glist, CREATE);
            menubutton_w_draw_inlets(x, glist, CREATE, 1,1 );
            menubutton_w_drawme(x, glist, DESTROY);
        }
        else
        {
            menubutton_w_draw_handle(x, glist, DESTROY);
            menubutton_w_draw_inlets(x, glist, DESTROY, 1, 1);
            menubutton_w_drawme(x, glist, CREATE);
        }
    }
    DEBUG(post("select end");)
}

static void menubutton_w_delete(t_gobj *z, t_glist *glist)
{
    DEBUG(post("delete start");)

    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist, x);

    DEBUG(post("delete end");)
}

       
static void menubutton_w_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_menubutton* s = (t_menubutton*)z;
    t_rtext *y;
    DEBUG(post("vis start");)
    DEBUG(post("vis: %d",vis);)
    if (vis) 
    {
#ifdef PD_MINOR_VERSION
     // JMZ: create an editor if there is none; 
      // on 0.42 there might be none IF [popup] is contained within a gop
      
#if PD_MINOR_VERSION > 41
      canvas_create_editor(glist);
#endif
      	y = (t_rtext *) rtext_new(glist, (t_text *)z);
#else
        y = (t_rtext *) rtext_new(glist, (t_text *)z, 0, 0);
#endif
        menubutton_w_drawme(s, glist, CREATE);
    }
    else 
    {
        y = glist_findrtext(glist, (t_text *)z);
        menubutton_w_drawme(s, glist, DESTROY);
        rtext_free(y);
    }
    DEBUG(post("vis end");)
}

t_widgetbehavior   menubutton_widgetbehavior = {
    w_getrectfn:  menubutton_w_getrect,
    w_displacefn: menubutton_w_displace,
    w_selectfn:   menubutton_w_select,
    w_activatefn: NULL,
    w_deletefn:   menubutton_w_delete,
    w_visfn:      menubutton_w_vis,
    w_clickfn:    NULL,
    //w_clickfn:    (t_clickfn)menubutton_w_newclick,
}; 
