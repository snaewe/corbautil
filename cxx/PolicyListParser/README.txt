The "PoaUtility" class used to have a built-in parser for POA-specific
policies. However, CORBA's use of policies is not restricted to the
creation of POAs. This directory contains some work-in-progress classes
that generalize the parsing of strings that contain policies. Currently,
the classes in this directory are used internally by the PoaUtility
class, but once they have matured (and also been implemented in Java)
they will be documented so they can be used in their own right.
