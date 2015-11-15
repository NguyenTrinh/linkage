# The Release System #

Linkage should follow the common [numeric](http://en.wikipedia.org/wiki/Software_versioning#Numeric) **major.minor.build** system that most open source projects commonly follow.

## Stable v Unstable ##

Even numbers represent a stable branch. These should be packaged and available for all major packaging systems.

The unstable or development branch releases should also be packaged and aim to support upcoming releases of Linux flavors or required software.  The last version of an unstable release should be identical to the first version of a stable release.

## Numbering System ##

### Major ###

The major version will only be increased once a significant set of features have been added.  This set of features are predefined in the RoadMap.

### Minor ###

The minor version is the most significant of the numbers in terms of applied meaning.  Even numbers imply stable releases and odd numbers imply development releases.  Releases occur on specific conditions, and guarantee no backward compatibility in terms of libraries used. i.e. if version 1.2.0 uses libtorrent-X and version 1.3.0 requires a newer version.

Minor version can be increased by one or more of the following factors:
  * A major Linux distribution release (Ubuntu/Fedora/Gentoo)
  * The release of a new libtorrent version
  * The addition of a new requirement (ie we start using lib-blah)
  * New functionality is introduced

### Build ###

Build versions increase when changes have been made that dont affect compatibility. Most bug fixes and security fixes fall into this category.  Above all, build versions of stable releases should be invisible to the user.  A gutsy user should be able to go from 2.4.12 to 2.4.18 without having to upgrade or install anything else.