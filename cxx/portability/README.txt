The files that have a "p_" prefix are a portability abstraction.
The portability is for building applications with several CORBA
products. At the moment, the following CORBA products are supported:

	o Orbix 5.x, 6.x
	o Orbacus 4.1.x
	o TAO 1.3 onwards
	o omniORB 4.0.3

It should be straight-forward to add support for other CORBA products.

The portability abstraction is also for #include-ing of "old" header
files, such as <iostream.h>, and the newer (standard) replacements,
such as <iostream>.

To make use of this portability abstraction library, you need to
#define some of the following...

If you are using Orbix then            #define P_USE_ORBIX
If you are using Orbacus then          #define P_USE_ORBACUS
If you are using TAO then              #define P_USE_TAO
If you are using omniORB then          #define P_USE_OMNIORB
If you are on Windows then             #define WIN32
If you use "old" C++ header files then #defined P_USE_OLD_TYPES

Documentation for the portability header files can be found in the
"Portability of C++ CORBA Applications" chapter in the
"doc/corba_utils.pdf" file.
