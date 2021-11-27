# calc_name
A failed attempt to replace spd_t::calc_name

Trying to find an fmt-based replacement to the way
spg_t calc_name() is implemeted.

Unfortunately, while the current implementation (which is based
on ritoa (in ceph's strtol.h) does not look very nice - it is
extremely fast. The alternatives in main.cpp were 4 times slower.
