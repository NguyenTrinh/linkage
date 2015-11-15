# Introduction #
Use this page to put up ideas for changes in the code base for the rewrite, and whatever new problems that arises with a Client-Server model and how to solve those etc.

# Server / Client #
_list interfaces that we need to export through xmlrpc_

# Config Settings #

All settings and config data should be migrated from gconf to simple classes (could this be made easier somehow?) that are serialized to disk via boost::serialization

```
class config
{
public:
  int int_setting;
  std::string string_setting;
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar && int_setting && string_setting;
  } 
}

int main()
{
  //load the config file
  config c;
  std::ifstream ifs(m_config_file.c_str(), std::ios::binary);
  boost::archive::text_iarchive ia(ifs);
  ia >> c;
}
```

This is just a basic example. We might want to make the members private and provide accessors or break the classes up a bit more like they are in gconf. or each component that requires settings simply creates its own config class and registers it with the SettingsManager. would be nice for plugin development (but might make things messy otherwise?)

# Library #

### Merge SessionManager and TorrentManager ###
These two are currently too intertwined, a merge makes sense in this regard.

  * Remove all complex cruft, keeping it simple is the key.
  * Move self contained stuff to stand alone classes which would be easier to extend in time, like a TorrentQueue, EventQueue.

### The Torrent class ###
This class contains most of the "weak" code, because we're wrapping so much of the functionallity of libtorrent's torrent\_handle in helper.

Therefore we should really remove alot of the current helpers, just keep whats really needed and makes sense. Advanced functionallity should be done through the torrent\_handle.

### Plugins ###
Should plugins live at the Server side at all? Yes.
Keeping them only on Client side would mean we would have to export a whole lot of functionality through xmlrpc, and Server - Client traffic would increase. Also making something like an ipfilter or scheduler plugin would become more complicated because either the client would always have to be running to control it or the deamon would have to store some plugin specific data which makes it not really a plugin at all.


A plugin living on the Server side would probably have to register some functionality to the core so that it can be called/notified from the Client side.  Its config settings should exposed for it automatically.

# UI #

### Client API ###

It might be useful to break out some of the client side functionality that would be common to both the web and gtk interfaces. For example:

  * sorting and filtering the torrent list
  * access to settings that would be common between both clients(is there any?)

### TreeView wrapper ###

Not even sure if this is possible but it would be sooo nice to make a simple listview without having to write 100 lines of code.

### WebUI ###

We might be better to wait until after the rewrite to do this, however doing it at the same time might help reveal any potential design flaws.  Especially in the design of the client API.