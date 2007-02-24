/*
Copyright (C) 2006	Christian Lundgren

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

#ifndef GROUP_FILTER_HH
#define GROUP_FILTER_HH

#include "linkage/Torrent.hh"
#include "linkage/WeakPtr.hh"

using namespace libtorrent;

class GroupFilter
{
public:
	/* FIXME: Add EVAL_LESS and EVAL_GREATER? */
	enum EvalType { EVAL_EQUALS, EVAL_CONTAINS, EVAL_STARTS, EVAL_ENDS };
	/* FIXME: Add more tags, like size, num_files etc.. */
	/* FIXME: Add more tags for dynamic filters, like % completed, share ratio etc.. */
	enum TagType { TAG_TRACKER, TAG_NAME, TAG_COMMENT, TAG_STATE };

	bool eval(const WeakPtr<Torrent>& torrent);

	const Glib::ustring& get_name();
	const Glib::ustring& get_filter();
	const TagType get_tag();
	const EvalType get_eval();

	GroupFilter(const Glib::ustring& filter, TagType tag, EvalType eval, const Glib::ustring& name);
	~GroupFilter();

protected:
	Glib::ustring m_filter;
	TagType m_tag;
	EvalType m_eval;
	Glib::ustring m_name;
};

#endif /* GROUP_FILTER_HH */
