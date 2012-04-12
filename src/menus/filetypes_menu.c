/* vifm
 * Copyright (C) 2001 Ken Steen.
 * Copyright (C) 2011 xaizek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <string.h> /* strdup() strlen() */

#include "../modes/menu.h"
#include "../utils/macros.h"
#include "../utils/string_array.h"
#include "../file_magic.h"
#include "../filelist.h"
#include "../filetype.h"
#include "../running.h"
#include "../ui.h"
#include "menus.h"

#include "filetypes_menu.h"

static const char * form_filetype_menu_entry(assoc_record_t prog,
		int descr_width);
static const char * form_filetype_data_entry(assoc_record_t prog);

int
show_filetypes_menu(FileView *view, int background)
{
	static menu_info m;

	int i;
	int max_len;

	char *filename = get_current_file_name(view);
	assoc_records_t ft = get_all_programs_for_file(filename);
	assoc_records_t magic = get_magic_handlers(filename);

	if(ft.count == 0 && magic.count == 0)
	{
		(void)show_error_msg("Filetype is not set.",
				"No programs set for this filetype.");
		return 0;
	}

	m.top = 0;
	m.current = 1;
	m.len = 0;
	m.pos = 0;
	m.hor_pos = 0;
	m.win_rows = 0;
	m.type = FILETYPE;
	m.matching_entries = 0;
	m.matches = NULL;
	m.match_dir = NONE;
	m.regexp = NULL;
	m.title = strdup(" Filetype associated commands ");
	m.args = NULL;
	m.items = NULL;
	m.data = NULL;
	m.extra_data = (background ? 1 : 0);
	m.key_handler = NULL;

	getmaxyx(menu_win, m.win_rows, i);

	max_len = 0;
	for(i = 0; i < ft.count; i++)
		max_len = MAX(max_len, strlen(ft.list[i].description));
	for(i = 0; i < magic.count; i++)
		max_len = MAX(max_len, strlen(magic.list[i].description));

	for(i = 0; i < ft.count; i++)
	{
		(void)add_to_string_array(&m.data, m.len, 1,
				form_filetype_data_entry(ft.list[i]));
		m.len = add_to_string_array(&m.items, m.len, 1,
				form_filetype_menu_entry(ft.list[i], max_len));
	}

	free(ft.list);

	(void)add_to_string_array(&m.data, m.len, 1,
			form_filetype_data_entry(NONE_PSEUDO_PROG));
	m.len = add_to_string_array(&m.items, m.len, 1, "");

	for(i = 0; i < magic.count; i++)
	{
		(void)add_to_string_array(&m.data, m.len, 1,
				form_filetype_data_entry(magic.list[i]));
		m.len = add_to_string_array(&m.items, m.len, 1,
				form_filetype_menu_entry(magic.list[i], max_len));
	}

	setup_menu();
	draw_menu(&m);
	move_to_menu_pos(m.pos, &m);
	enter_menu_mode(&m, view);

	return 0;
}

/* Returns pointer to a statically allocated buffer */
static const char *
form_filetype_menu_entry(assoc_record_t prog, int descr_width)
{
	static char result[PATH_MAX];
	if(descr_width > 0)
	{
		char format[16];
		if(prog.description[0] == '\0')
		{
			snprintf(format, sizeof(format), " %%-%ds  %%s", descr_width);
		}
		else
		{
			snprintf(format, sizeof(format), "[%%-%ds] %%s", descr_width);
		}
		snprintf(result, sizeof(result), format, prog.description, prog.command);
	}
	else
	{
		snprintf(result, sizeof(result), "%s", prog.command);
	}
	return result;
}

/* Returns pointer to a statically allocated buffer */
static const char *
form_filetype_data_entry(assoc_record_t prog)
{
	static char result[PATH_MAX];
	snprintf(result, sizeof(result), "%s|%s", prog.description, prog.command);
	return result;
}

void
execute_filetype_cb(FileView *view, menu_info *m)
{
	if(view->dir_entry[view->list_pos].type == DIRECTORY && m->pos == 0)
	{
		handle_dir(view);
	}
	else
	{
		const char *prog_str = strchr(m->data[m->pos], '|') + 1;
		if(prog_str[0] != '\0')
		{
			int background = m->extra_data & 1;
			run_using_prog(view, prog_str, 0, background);
		}
	}

	clean_selected_files(view);
	draw_dir_list(view, view->top_line);
	move_to_list_pos(view, view->list_pos);
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */