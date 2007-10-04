The files in this directory provide Orbix 3-specific implementations of
two utility functions called corbautil::importObjRef() and
corbautil::exportObjRef(). These functions make it very easy for CORBA
applications to decide at runtime how they want to import or export
object references.

Note that Orbix 3 does *not* support "corbaloc" or "corbaname" URLs, so
they cannot be passed as parameters to importObjRef().
