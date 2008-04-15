/*
Copyright (C) 2008	Dave Moore

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

#ifndef SCHEDULE_HH
#define SCHEDULE_HH

#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <iostream>

typedef std::pair<int, int> TimeSlot; // <day, hour>

class Limit
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & up;
		ar & down;
	}
public:
	int up;
	int down;
	Limit() : up(0), down(0) {}
	Limit(int u, int d) : up(u), down(d) {}
};

class Schedule
{
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & limits;
	    ar & schedule;
	    ar & lclick;
	    ar & rclick;
	    ar & name;
	}

public:
	Limit limits[3];
	int schedule[7][24];
	int lclick;
	int rclick;
	std::string name;
	
	Schedule() : name("default"), lclick(0), rclick(0)
	{
		// initialize the schedule array to 0s
		for (int i=0; i<7*24; i++) { schedule[i%7][i/7] = 0; }
		//for (int c=0; c < 7; c++)
		//{
			//for (int r=0; r < 24; r++)
			//{
				//schedule[c][r] = 0;
			//}
		//}
	}
	Schedule(Glib::ustring _name) : name(_name), lclick(0), rclick(0) 
	{ for (int i=0; i<7*24; i++) { schedule[i%7][i/7] = 0; } }
};

class SchedulerConfig
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & current_schedule;
		ar & schedules;
	}
public:
	int current_schedule;
	std::vector<Schedule> schedules;

	Schedule & get_current_schedule() 
	{ 
		return schedules[current_schedule];
	};

	SchedulerConfig(const int cur) : current_schedule(cur) {};
	SchedulerConfig() : current_schedule(0) {}
	~SchedulerConfig() {}
};


#endif //SCHEDULE_HH
