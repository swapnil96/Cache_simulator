# Cache_simulator

Assumptions

1) As write allocate option was not given in the CFG file
a) A write-back cache uses write allocate, hoping for subsequent writes (or even reads) to the same location, which is now cached.
b) A write-through cache uses no-write allocate. Here, subsequent writes have no advantage, since they still need to be written directly to the backing store.


