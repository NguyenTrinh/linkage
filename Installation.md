**_This page contains alot of diffrent instructions, this should be split into two separate pages. Something like GetLinkage (general information about packages and how to install them) and BuildingLinkage (detailed information on how to build linkage, options and deps)_**


# Fedora #

## Binrary packages ##
Linkage 0.1.5 is available from the downloads page, the packages you need are linkage, libtorrent and libdbusc++. All are available for download there, the -devel and -debuginfo packages are not needed in general.

Thanks drago01 Linkage 0.1.4 is now available in Fedora repos.
```
yum --enablerepo=development install linkage
```

## Source install ##
**This info is for 0.1.4**
Optional dependencies are curl, gupnp, exo, libgnomemm, gnome-vfsmm and libgnomeuimm.

Exo is for Xfce session support, the GNOME libraries are for GNOME session support. You can specify which optional dependencies you want to build with by passing --with-DEP or --without-DEP to the configure script (e.g. ./configure --with-exo --without-gnome --with-curl --without-gupnp)

(You will need the -devel packages to build linkage, however after installation you can safely remove the -devel packages if you wish)
```
yum -y install libtool autoconf automake gcc-c++
yum -y install rb_libtorrent-devel gtkmm24-devel libnotify-devel dbus-glib-devel gconfmm26-devel libglademm24-devel
yum -y install curl-devel exo-devel libgnomemm26-devel gnome-vfsmm26-devel libgnomeuimm26-devel
wget http://linkage.googlecode.com/files/linkage-0.1.4.tar.gz
tar xzvpf linkage-0.1.4.tar.gz
cd linkage-0.1.4
./configure --prefix=/usr
make
su -c 'make install'
```

# Ubuntu (Gutsy Gibbon) #
Ubuntu packages are available from the download page, the packages you need are linkage, libtorrent and libdbusc++. All are available for download there, both for amd64 and i386.

## Source install ##
**This info is for 0.1.4**
Currently there are no packages for rb\_libtorrent nor linkage for Ubuntu. So if you want to install linkage you have to build them both from source.

Libtorrent can be downloaded [here](http://sourceforge.net/project/downloading.php?group_id=79942&filename=libtorrent-0.12.tar.gz).

```
sudo apt-get install build-essential libtool autoconf automake
apt-get install libboost-date-time-dev libboost-date-time1.34.1 \
libboost-filesystem-dev libboost-filesystem1.34.1 libboost-regex-dev \
libboost-regex1.34.1 libboost-signals-dev libboost-signals1.34.1 \
libboost-iostreams-dev libboost-iostreams1.34.1 libboost-thread-dev\
libboost-thread1.34.1 libboost-dev libboost-signals-dev
cd libtorrent-0.12
./configure --prefix=/usr
make
sudo make install
```

(I might have missed some package below, if you notice anything missing please let me know :)
```
sudo apt-get install libgtkmm-2.4-dev libnotify-dev libcurl4-openssl-dev \
libdbus-glib-1-dev libgtkmm-2.4-1c2a libnotify1 libcurl3 libdbus-glib-1-2 \
libgnomemm-2.6-1c2a libgnomemm-2.6-dev libgnomeuimm-2.6-1c2a libgnomeuimm-2.6-dev \
libgnome-vfsmm-2.6-1c2a libgnome-vfsmm-2.6-dev libexo-0.3-0 libexo-0.3-dev \
libgconfmm-2.6-1c2 libgconfmm-2.6-dev libglademm-2.4-1c2a libglademm-2.4-dev
wget http://linkage.googlecode.com/files/linkage-0.1.4.tar.gz
tar xzvpf linkage-0.1.4.tar.gz
cd linkage-0.1.4
./configure --prefix=/usr
make
sudo make install
```


# Gentoo #
An ebuild is available in Portage, http://packages.gentoo.org/packages/?category=net-p2p;name=linkage


# Install from SVN #
Currently you will need dbus-c++ to build Linkage from svn. There has not been any formal release of dbus-c++ yet so you have to grab it from git. If you are using Fedora you can use the RPMs available in the Downloads section.
```
git clone git://anongit.freedesktop.org/git/dbus/dbus-c++/
cd dbus-c++
./autogen.sh
./configure --prefix=/usr --enable-glib
make
su -c 'make install'
```

Other than that the dependencies are the same as for 0.1.4, except that you now need libgtop and that gupnp and curl are no longer used. You also need libtorrent 0.13 (which is unreleased at the time so grab it from svn).
```
 svn co https://libtorrent.svn.sourceforge.net/svnroot/libtorrent/branches/RC_0_13/ libtorrent
```
Then follow rest of instructions here: http://www.rasterbar.com/products/libtorrent/building.html#building-from-svn