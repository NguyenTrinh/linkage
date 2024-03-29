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

#ifndef GROUP_HH
#define GROUP_HH

#include <boost/intrusive_ptr.hpp>
#include "libtorrent/intrusive_ptr_base.hpp"

#include "linkage/Torrent.hh"

class Group;

typedef boost::intrusive_ptr<Group> GroupPtr;

class Group : public libtorrent::intrusive_ptr_base<Group>
{
public:
	/* FIXME: Add EVAL_LESS and EVAL_GREATER? */
	enum EvalType { EVAL_EQUALS, EVAL_CONTAINS, EVAL_STARTS, EVAL_ENDS };
	/* FIXME: Add more tags, like size, num_files etc.. */
	/* FIXME: Add more tags for dynamic filters, like % completed, share ratio etc.. */
	enum TagType { TAG_TRACKER, TAG_NAME, TAG_COMMENT };

	enum OperationType { OP_OR, OP_NOR, OP_AND, OP_NAND };

	struct Filter
	{
		Glib::ustring filter;
		TagType tag;
		EvalType eval;

		/* Note to self, first filter _must_ always be OP_OR or OP_NOR */
		OperationType operation;

		Filter(const Glib::ustring& f, TagType t, EvalType e, OperationType o) : 
			filter(f),
			tag(t),
			eval(e),
			operation(o) {}
	};

	bool eval(const Linkage::TorrentPtr& torrent);

	const Glib::ustring& get_name() const;
	const std::list<Filter>& get_filters() const;

	Group(const Glib::ustring& name, const std::list<Filter>& filters);
	Group();
	~Group();

private:
	std::list<Filter> m_filters;
	Glib::ustring m_name;

	std::map<libtorrent::sha1_hash, bool>* m_statics;
};

#endif /* GROUP_HH */
