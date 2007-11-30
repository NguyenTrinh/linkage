/*
Copyright (C) 2006-2007   Christian Lundgren
Copyright (C) 2007        Dave Moore

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA	02110-1301, USA.
*/

#include <gtkmm/button.h>
#include <glibmm/i18n.h>

#include "linkage/Utils.hh"
#include "GroupsWin.hh"

using namespace Linkage;

GroupsWin::GroupsWin(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::Window(cobject),
	glade_xml(refGlade)
{
	glade_xml->get_widget_derived("edit_dialog", group_edit);
	group_edit->set_transient_for(*this);

	glade_xml->get_widget("groups_treeview", groups_view);

	model = Gtk::ListStore::create(columns);
	groups_view->set_model(model);

	groups_view->append_column(_("Name"), columns.name);
	Gtk::TreeViewColumn* column = groups_view->get_column(0);
	Gtk::CellRenderer* renderer = column->get_first_cell_renderer();
	column->clear_attributes(*renderer);
	column->add_attribute(*renderer, "markup", 0);
	column->set_expand(true);
	column->set_cell_data_func(*renderer, sigc::mem_fun(this, &GroupsWin::format_name));

	Gtk::Button* close_button = NULL;
	glade_xml->get_widget("groups_close", close_button);
	close_button->signal_clicked().connect(sigc::mem_fun(this, &GroupsWin::on_button_close));

	Gtk::Button* new_button = NULL;
	glade_xml->get_widget("groups_new", new_button);
	new_button->signal_clicked().connect(sigc::mem_fun(this, &GroupsWin::on_button_new));

	Gtk::Button* remove_button = NULL;
	glade_xml->get_widget("groups_remove", remove_button);
	remove_button->signal_clicked().connect(sigc::mem_fun(this, &GroupsWin::on_button_remove));

	Gtk::Button* edit_button = NULL;
	glade_xml->get_widget("groups_edit", edit_button);
	edit_button->signal_clicked().connect(sigc::mem_fun(this, &GroupsWin::on_button_edit));

	/* Load groups data from disk */
	libtorrent::entry e;
	if (load_entry(Glib::build_filename(get_config_dir(), "groups"), e))
	{
		// for each group
		for (libtorrent::entry::dictionary_type::iterator i = e.dict().begin();
			i != e.dict().end(); ++i)
		{
			libtorrent::entry::list_type e_filters = i->second.list();

			std::list<Group::Filter> filters;			
			// for each filter
			for (libtorrent::entry::list_type::iterator j = e_filters.begin();
				j != e_filters.end(); ++j)
			{
				libtorrent::entry::dictionary_type e_filter = j->dict();
				Glib::ustring filter = e_filter["filter"].string();
				Group::EvalType eval = Group::EvalType(e_filter["eval"].integer());
				Group::TagType tag = Group::TagType(e_filter["tag"].integer());
				Group::OperationType op = Group::OperationType(e_filter["operation"].integer());
				filters.push_back(Group::Filter(filter, tag, eval, op));
			}

			Gtk::TreeRow row = *(model->append());
			row[columns.name] = i->first;
			row[columns.filters] = filters;
		}
	}
}

GroupsWin::~GroupsWin()
{
	libtorrent::entry::dictionary_type e_groups;

	/* For each group row */
	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter i = children.begin(); i != children.end(); ++i)
	{
		Gtk::TreeRow row = *i;
		Glib::ustring name = row[columns.name];
		std::list<Group::Filter> filters = row[columns.filters];

		/* For each filter */
		libtorrent::entry::list_type e_filters;
		for (std::list<Group::Filter>::iterator j = filters.begin();
			j != filters.end(); ++j)
		{
			Group::Filter filter = *j;
			libtorrent::entry::dictionary_type e_filter;
			
			e_filter["filter"] = libtorrent::entry(filter.filter);
			e_filter["eval"] = libtorrent::entry(filter.eval);
			e_filter["tag"] = libtorrent::entry(filter.tag);
			e_filter["operation"] = libtorrent::entry(filter.operation);

			/* Pack each filter in list */
			e_filters.push_back(e_filter);
		}
		/* Store filter list in dictionary */
		e_groups[name] = e_filters;
	}

	save_entry(Glib::build_filename(get_config_dir(), "groups"), e_groups);
}

sigc::signal<void, const std::list<Group>& > GroupsWin::signal_groups_changed()
{
	return m_signal_groups_changed;
}

void GroupsWin::on_button_remove()
{
	Gtk::TreeIter iter = groups_view->get_selection()->get_selected();
	if (iter)
	{
		model->erase(iter);
	}
}

void GroupsWin::on_button_edit()
{
	Gtk::TreeIter iter = groups_view->get_selection()->get_selected();
	if (iter)
	{
		Gtk::TreeRow row = *iter;
		Glib::ustring name = row[columns.name];
		std::list<Group::Filter> filters = row[columns.filters];

		group_edit->run(name, filters);

		row[columns.name] = name;
		row[columns.filters] = filters;
	}
}

void GroupsWin::on_button_new()
{
	Gtk::TreeRow row = *(model->append());

	Glib::ustring name;
	std::list<Group::Filter> filters;

	group_edit->run(name, filters);

	row[columns.name] = name;
	row[columns.filters] = filters;
}

bool GroupsWin::on_delete_event(GdkEventAny*)
{
	hide();

	return true;
}

void GroupsWin::on_button_close()
{
	hide();
}

void GroupsWin::on_hide()
{
	Gtk::Window::on_hide();

	notify();
}

void GroupsWin::notify()
{
	std::list<Group> groups;
	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		groups.push_back(Group(row[columns.name], row[columns.filters]));
	}
	m_signal_groups_changed.emit(groups);
}

void GroupsWin::format_name(Gtk::CellRenderer* renderer, const Gtk::TreeIter& iter)
{
	if (!iter)
		return;

	std::stringstream ss;
	ss.imbue(std::locale(""));

	Gtk::TreeRow row = *iter;

	ss << "<b>" << row[columns.name] << "</b>\n<small>";

	std::list<Group::Filter> filters = row[columns.filters];
	if (!filters.empty())
	{
		ss << _("If") << " ";

		std::list<Group::Filter>::iterator iter = filters.begin();
		while (iter != filters.end())
		{
			Group::Filter filter = *iter;
			/* Skip first and/or/.. */
			bool first = (iter == filters.begin());
			if (!first)
				ss << ", ";

			switch (filter.operation)
			{
				case Group::OP_AND:
					ss << _("and") << " ";
					break;
				case Group::OP_NAND:
					ss << _("and not") << " ";
					break;
				case Group::OP_OR:
					if (!first)
						ss << _("or") << " ";
					break;
				case Group::OP_NOR:
					if (!first)
						ss << _("or not") << " ";
					else
						ss << _("not") << " ";
					break;
			}

			switch (filter.tag)
			{
				case Group::TAG_COMMENT:
					ss << _("comment") << " ";
					break;
				case Group::TAG_TRACKER:
					ss << _("tracker") << " ";
					break;
				case Group::TAG_NAME:
					ss << _("name") << " ";
					break;
			}

			switch (filter.eval)
			{
				case Group::EVAL_EQUALS:
					ss << _("equals") << " ";
					break;
				case Group::EVAL_CONTAINS:
					ss << _("contains") << " ";
					break;
				case Group::EVAL_STARTS:
					ss << _("starts with") << " ";
					break;
				case Group::EVAL_ENDS:
					ss << _("ends with") << " ";
					break;
			}

			ss << filter.filter;

			++iter;
		}
	}
	ss << "</small>";

	Gtk::CellRendererText* txt_renderer = dynamic_cast<Gtk::CellRendererText*>(renderer);
	txt_renderer->property_markup() = Glib::locale_to_utf8(ss.str());
}

