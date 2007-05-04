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

#ifndef WEAKPTR_HH
#define WEAKPTR_HH

#include <cassert>

/* 
		This is used to make handling with Torrent pointers a little bit safer,
		since this wrapper class makes it harder to delete the underlying object.
		
		Explanation:
		
		Torrent* t = TorrentManager->get_torrent(hash);
		t->some_method_that_alters_t();
		delete t; // We don't want this to be possible, or at least not as easy.
		
		Solution:
		
		WeakPtr<Torrent> t =	TorrentManager->get_torrent(hash);
		t->some_method();
*/
		
template<class T> class WeakPtr
{
	T* m_object;
	
public:
	T* operator->() const
	{
		assert(m_object != NULL);
		return m_object;
	}
	
	operator bool() const
	{
		return (m_object != NULL);
	}

	bool operator==(const WeakPtr<T>& src) const
	{
		return (m_object == src.m_object);
	}

	bool operator!=(const WeakPtr<T>& src) const
	{
		return (m_object != src.m_object);
	}
	
	WeakPtr(T* object)
	{
		m_object = object;
	};
	
	WeakPtr()
	{
		m_object = NULL;
	};
	
	~WeakPtr() {};
};

#endif /* WEAKPTR_HH */
