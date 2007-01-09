/*
Copyright (C) 2006  Christian Lundgren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef REF_COUNTER_HH
#define REF_COUNTER_HH

template<class T> class RefCounter
{
private:
	int references;
	T* m_object;
	
	void try_clean()
	{
		static bool deleteing = false;
		if (m_object != NULL && !references && !deleteing) 
		{
			deleteing = true;
			delete m_object;
			deleteing = false;
		}
	};
public:
	void reference() 
	{ 
		references++; 
	};
	
	void unreference() 
	{ 
		references--; 
		try_clean();
	};
	
	RefCounter(T* object) 
	{
		m_object = object; 
		references = 1; 
	};
	
	virtual ~RefCounter() 
	{ 
		/* FIXME: This is useless? */
		try_clean();
	};
};

#endif /* REF_COUNTER_HH */
