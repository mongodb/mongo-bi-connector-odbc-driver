# #########################################################
# 
#
# \brief
#	This is a qmake project file for building when Qt
#       is being used to build GUI bits. 
#
#	The GUI bits are;
#
#	setup   	- Setup library which handles
#			requests from the; ODBC Administrator,
#			driver, or application for GUI 
#			prompting.
#	dsn-editor	- Setup application for when
#			the systems regular ODBC 
#			Administrator application is not
#			making one happy (on OSX perhaps).
#
#	This project file is used to build and install 
# 	Qt GUI bits of Connector/ODBC. Basically; if
#	you have Qt development files installed then you 
#	should have the qmake utility. The qmake utility 
#	can create a regular 'Makefile' from this project
#	file.
#
#	Using qmake instead of the GNU auto-tools gets
#	around problems building certian features, and
#	simplifies maintainence as these project files
#	are much easier to work with. Using qmake also
#	increases the chance that we get all the correct
#	compile and link options for Qt based code.
#	
#	Unfortunately; this is not hooked into the GNU
#	auto-tools at this time so one must build/install
#	the driver code with that first and then, 
#	optionally do the following to build the Qt based
#	GUI bits;
#	
#	$ qmake root.pro
#	$ make
#
# \note OSX
#	
#	As of Qt4, qmake will generate xcode project files
#	by default. So to get regular make files one should;
#
#	qmake -spec macx-g++
#
# \note
#
#	1. The qmake approach was inspired by spending way
#	too many hours trying to get this stuff integrated
#	into GNU auto-build only to find that it fails
#	on OSX when building setup library as a bundle.
#	Seems libtool does not take the framework link
#	options and subsequently leaves unresolved refs
#	(as reported by dltest). 
#
#	2. You only need to use this way to make gui bits if
#	the gnu method of building the gui bits is 
#	failing you (on OSX for example) or if the platform
#	does not have gnu auto-build (Windows perhaps).
#
#       3. The util code does not contain any Qt but it should
#       be built with qmake if you are going to use Qt 
#       elsewhere.
#
# \sa
#       Build.bat
#       Install.bat
#       Uninstall.bat
#       Upgrade.bat
#	http://www.trolltech.com
# 
# #########################################################
TEMPLATE        = subdirs
SUBDIRS         = \
                MYODBCDbg \
                dltest \
		util \
		setup \
                installer \
                monitor \
		dsn-editor \
                driver \
                test
