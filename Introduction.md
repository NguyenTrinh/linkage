# Introduction #

Linkage is a BitTorrent client written in C++. It based on libtorrent and uses the GTK+ toolkit (through gtkmm).

# Purpose #

At the time I started writing Linkage there wasn't any decent BitTorrent client around for the GTK+ desktop.

I guess Dave had a similar view of things when he started his own client, I saw it and suggested that we merged our efforts. The result is Linkage.

# Screenshots #

| ![![](http://zeflunk.googlepages.com/linkage-small.th.png)](http://zeflunk.googlepages.com/linkage-small.png) | ![![](http://zeflunk.googlepages.com/linkage-general.th.png)](http://zeflunk.googlepages.com/linkage-general.png) | ![![](http://zeflunk.googlepages.com/linkage-transfer.th.png)](http://zeflunk.googlepages.com/linkage-transfer.png) | ![![](http://zeflunk.googlepages.com/linkage-files.th.png)](http://zeflunk.googlepages.com/linkage-files.png) |
|:--------------------------------------------------------------------------------------------------------------|:------------------------------------------------------------------------------------------------------------------|:--------------------------------------------------------------------------------------------------------------------|:--------------------------------------------------------------------------------------------------------------|

| ![![](http://zeflunk.googlepages.com/linkage-prefs.th.png)](http://zeflunk.googlepages.com/linkage-prefs.png) | ![![](http://zeflunk.googlepages.com/linkage-groups.th.png)](http://zeflunk.googlepages.com/linkage-groups.png) | ![![](http://zeflunk.googlepages.com/linkage-new.th.png)](http://zeflunk.googlepages.com/linkage-new.png) |
|:--------------------------------------------------------------------------------------------------------------|:----------------------------------------------------------------------------------------------------------------|:----------------------------------------------------------------------------------------------------------|

Some more screenshots, and review is available here (in Chinese):  http://linuxtoy.org/archives/linkage.html

# Installing #
See [Installation](Installation.md)

## Running ##

You can start Linkage either by typing linkage in a terminal or simply clicking the launcher in your menu.

If you just installed Linkage you might need to run the following as root to make it show up in the menu:
```
update-desktop-database /usr/share/applications
```











