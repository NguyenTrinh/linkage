# Introduction #
Linkage exports two DBus interfaces, the first one is org.linkage.Interface which can be used to interact with the running application. Such as adding torrents, showing/hiding the main window. The object path for this interface is /org/linkage/interface.

The second interface is org.linkage.Torrent, this one is pretty obvious. Through this interface you can start, stop and remove individual torrents, as well as get information about it's current state and progress. The object path for a torrent is /org/linkage/torrents/$hash, where $hash is the torrent's unique SHA1 hash.

# Example #
Here is a sample python script that demonstrates some information you can retrive from the org.linkage.Torrent interface:
```
#!/usr/bin/env/python
import dbus
from xml.parsers.expat import ParserCreate

# this list holds all object paths
torrents = []

# get the object paths from dbus
def parseNames(name, attributes):
	if attributes.has_key("name"):
		torrents.append("/org/linkage/torrents/" + attributes["name"])

bus = dbus.SessionBus()
nodes = bus.get_object("org.linkage", "/org/linkage/torrents").Introspect()

parser = ParserCreate('UTF-8', ' ')
parser.buffer_text = True
parser.StartElementHandler = parseNames
parser.EndElementHandler = None
parser.Parse(nodes)

# sort on position =)
sort_map = {}
for path in torrents:
	obj = bus.get_object("org.linkage", path)
	sort_map[path] = obj.GetPosition(dbus_interface="org.linkage.Torrent")

def cmpPosition(path1, path2):
	return int(sort_map[path1] - sort_map[path2])

torrents.sort(cmpPosition)

print "for each"
# dump some info for each torrent
for path in torrents:
	# get our torrent proxy object
	obj = bus.get_object("org.linkage", path)
	t = dbus.Interface(obj, dbus_interface="org.linkage.Torrent")
	# get info from linkage process
	name = t.GetName()
	position = t.GetPosition()
	print "(%i) %s" % (position, name)
	# some more for fun
	print "\tState: %s" % t.GetState()
	print "\tProgress: %i" % (t.GetProgress() * 100)
	(downloaded, uploaded) = t.GetTransfered()
	(down_rate, up_rate) = t.GetRates()
	print "\tDownloaded: %iMb, Rate: %iKb/s" % (downloaded/(1024.0*1024), down_rate/(1024.0*1024))
	print "\tUploaded: %iMb, Rate: %iKb/s" % (uploaded/(1024.0*1024), up_rate/(1024.0*1024))

```