# Tarball and others #

There are a couple basic steps needed before making a new release:

  * Get a fresh update of trunk, copy it to a new tag (tags/linkage-X.Y.Z).
  * Run autogen.sh and edit version information in configure.ac, add configure.ac and delete configure.ac.in. Also remove the sed line in autogen.sh.
  * Update ChangeLog for release.
  * Rerun autogen.sh and run make dist/make distcheck
  * Upload generated tarball, mark old release as deprecated and new as featured.
  * Make a formal announcement on the mailing list.
  * Update wiki pages if needed.
  * Do all the stuff I forgot to write here.

# Debs #

# RPMs #

The linkage package for fedora is currently maintained by some other nice guy. However we should provide fresh rpms for linkage upon release (and also libtorrent and dbus-c++ currently).