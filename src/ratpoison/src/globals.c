/* Copyright (C) 2000, 2001, 2002, 2003 Shawn Betts
 *
 * This file is part of ratpoison.
 *
 * ratpoison is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * ratpoison is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 */

#include "ratpoison.h"

int alarm_signalled = 0;
int kill_signalled = 0;
int hup_signalled = 0;
int chld_signalled = 0;
int rat_x;
int rat_y;
int rat_visible = 1;        /* rat is visible by default */

Atom wm_name;
Atom wm_state;
Atom wm_change_state;
Atom wm_protocols;
Atom wm_delete;
Atom wm_take_focus;
Atom wm_colormaps;

Atom rp_command;
Atom rp_command_request;
Atom rp_command_result;
Atom rp_selection;

int rp_current_screen;
rp_screen *screens;
int num_screens;
Display *dpy;

rp_group *rp_current_group;
LIST_HEAD (rp_groups);
LIST_HEAD (rp_children);
struct rp_defaults defaults;

int ignore_badwindow = 0;

char **myargv;

struct rp_key prefix_key;

struct modifier_info rp_modifier_info;

/* rudeness levels */
int rp_honour_transient_raise = 1;
int rp_honour_normal_raise = 1;
int rp_honour_transient_map = 1;
int rp_honour_normal_map = 1;

char *rp_error_msg = NULL;

/* Global frame numset */
struct numset *rp_frame_numset;

/* The hook dictionary globals. */

LIST_HEAD (rp_key_hook);
LIST_HEAD (rp_switch_win_hook);
LIST_HEAD (rp_switch_frame_hook);
LIST_HEAD (rp_switch_group_hook);
LIST_HEAD (rp_quit_hook);
LIST_HEAD (rp_restart_hook);

struct rp_hook_db_entry rp_hook_db[]=
  {{"key",      &rp_key_hook},
   {"switchwin",    &rp_switch_win_hook},
   {"switchframe",  &rp_switch_frame_hook},
   {"switchgroup",  &rp_switch_group_hook},
   {"quit",         &rp_quit_hook},
   {"restart",      &rp_restart_hook},
   {NULL, NULL}};

void
set_rp_window_focus (rp_window *win)
{
  if ( desktop_has_focus() == 1 && win != desktop_window )
  {
    PRINT_DEBUG (("The target window is not the desktop window and the desktop has the focus. Ignoring request!"));
    return;
  }

  PRINT_DEBUG (("Giving focus to '%s'\n", window_name (win)));
  int result = XSetInputFocus (dpy, win->w, RevertToPointerRoot, CurrentTime);
  PRINT_DEBUG (("Result of XSetInputFocus %d, %p\n", result, win->w));
}

void
set_window_focus (Window window)
{
  PRINT_DEBUG (("Giving focus to %ld\n", window));
  XSetInputFocus (dpy, window,
          RevertToPointerRoot, CurrentTime);
}
