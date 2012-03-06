User-space memory map
=====================

 * `[0x00000000..0x00120000]`, or similar: kernel code and static data space. PL=0?
 * `[0x0f000000..0x10000000]`: kernel heap; set the PL to 0.
 * `[..0x10000000]`: per-task kernel stack space. PL=0.
 * `[0x10000000..]`: user task image. PL=3.
 * `[EMPTY PAGE HERE]`: stack guard. *Present=0*; PL=3.
 * `[..+STACK_SIZE]`: user stack space. PL=3.
