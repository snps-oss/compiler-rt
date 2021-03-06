# This directory contains a large amount of C code which provides
# generic implementations of the core runtime library along with optimized
# architecture-specific code in various subdirectories.

set(GENERIC_SOURCES
  absvdi2.c
  absvsi2.c
  absvti2.c
  adddf3.c
  addsf3.c
  addvdi3.c
  addvsi3.c
  addvti3.c
  apple_versioning.c
  ashldi3.c
  ashlti3.c
  ashrdi3.c
  ashrti3.c
  # FIXME: atomic.c may only be compiled if host compiler understands _Atomic
  # atomic.c
  clear_cache.c
  clzdi2.c
  clzsi2.c
  clzti2.c
  cmpdi2.c
  cmpti2.c
  comparedf2.c
  comparesf2.c
  ctzdi2.c
  ctzsi2.c
  ctzti2.c
  divdc3.c
  divdf3.c
  divdi3.c
  divmoddi4.c
  divmodsi4.c
  divsc3.c
  divsf3.c
  divsi3.c
  divti3.c
  divxc3.c
  enable_execute_stack.c
  eprintf.c
  extendsfdf2.c
  ffsdi2.c
  ffsti2.c
  fixdfdi.c
  fixdfsi.c
  fixdfti.c
  fixsfdi.c
  fixsfsi.c
  fixsfti.c
  fixunsdfdi.c
  fixunsdfsi.c
  fixunsdfti.c
  fixunssfdi.c
  fixunssfsi.c
  fixunssfti.c
  fixunsxfdi.c
  fixunsxfsi.c
  fixunsxfti.c
  fixxfdi.c
  fixxfti.c
  floatdidf.c
  floatdisf.c
  floatdixf.c
  floatsidf.c
  floatsisf.c
  floattidf.c
  floattisf.c
  floattixf.c
  floatundidf.c
  floatundisf.c
  floatundixf.c
  floatunsidf.c
  floatunsisf.c
  floatuntidf.c
  floatuntisf.c
  floatuntixf.c
  gcc_personality_v0.c
  int_util.c
  lshrdi3.c
  lshrti3.c
  moddi3.c
  modsi3.c
  modti3.c
  muldc3.c
  muldf3.c
  muldi3.c
  mulodi4.c
  mulosi4.c
  muloti4.c
  mulsc3.c
  mulsf3.c
  multi3.c
  mulvdi3.c
  mulvsi3.c
  mulvti3.c
  mulxc3.c
  negdf2.c
  negdi2.c
  negsf2.c
  negti2.c
  negvdi2.c
  negvsi2.c
  negvti2.c
  paritydi2.c
  paritysi2.c
  parityti2.c
  popcountdi2.c
  popcountsi2.c
  popcountti2.c
  powidf2.c
  powisf2.c
  powitf2.c
  powixf2.c
  subdf3.c
  subsf3.c
  subvdi3.c
  subvsi3.c
  subvti3.c
  trampoline_setup.c
  truncdfsf2.c
  ucmpdi2.c
  ucmpti2.c
  udivdi3.c
  udivmoddi4.c
  udivmodsi4.c
  udivmodti4.c
  udivsi3.c
  udivti3.c
  umoddi3.c
  umodsi3.c
  umodti3.c)

set(x86_64_SOURCES
  x86_64/floatdidf.c
  x86_64/floatdisf.c
  x86_64/floatdixf.c
  x86_64/floatundidf.S
  x86_64/floatundisf.S
  x86_64/floatundixf.S
  ${GENERIC_SOURCES})

set(i386_SOURCES
  i386/ashldi3.S
  i386/ashrdi3.S
  i386/divdi3.S
  i386/floatdidf.S
  i386/floatdisf.S
  i386/floatdixf.S
  i386/floatundidf.S
  i386/floatundisf.S
  i386/floatundixf.S
  i386/lshrdi3.S
  i386/moddi3.S
  i386/muldi3.S
  i386/udivdi3.S
  i386/umoddi3.S
  ${GENERIC_SOURCES})

set(arm_SOURCES
  arm/adddf3vfp.S
  arm/addsf3vfp.S
  arm/aeabi_dcmp.S
  arm/aeabi_fcmp.S
  arm/aeabi_idivmod.S
  arm/aeabi_ldivmod.S
  arm/aeabi_memcmp.S
  arm/aeabi_memcpy.S
  arm/aeabi_memmove.S
  arm/aeabi_memset.S
  arm/aeabi_uidivmod.S
  arm/aeabi_uldivmod.S
  arm/bswapdi2.S
  arm/bswapsi2.S
  arm/comparesf2.S
  arm/divdf3vfp.S
  arm/divmodsi4.S
  arm/divsf3vfp.S
  arm/divsi3.S
  arm/eqdf2vfp.S
  arm/eqsf2vfp.S
  arm/extendsfdf2vfp.S
  arm/fixdfsivfp.S
  arm/fixsfsivfp.S
  arm/fixunsdfsivfp.S
  arm/fixunssfsivfp.S
  arm/floatsidfvfp.S
  arm/floatsisfvfp.S
  arm/floatunssidfvfp.S
  arm/floatunssisfvfp.S
  arm/gedf2vfp.S
  arm/gesf2vfp.S
  arm/gtdf2vfp.S
  arm/gtsf2vfp.S
  arm/ledf2vfp.S
  arm/lesf2vfp.S
  arm/ltdf2vfp.S
  arm/ltsf2vfp.S
  arm/modsi3.S
  arm/muldf3vfp.S
  arm/mulsf3vfp.S
  arm/nedf2vfp.S
  arm/negdf2vfp.S
  arm/negsf2vfp.S
  arm/nesf2vfp.S
  arm/restore_vfp_d8_d15_regs.S
  arm/save_vfp_d8_d15_regs.S
  arm/subdf3vfp.S
  arm/subsf3vfp.S
  arm/switch16.S
  arm/switch32.S
  arm/switch8.S
  arm/switchu8.S
  arm/sync_synchronize.S
  arm/truncdfsf2vfp.S
  arm/udivmodsi4.S
  arm/udivsi3.S
  arm/umodsi3.S
  arm/unorddf2vfp.S
  arm/unordsf2vfp.S
  ${GENERIC_SOURCES})

add_custom_target(builtins)

if (NOT WIN32)
  foreach(arch x86_64 i386 arm)
    if(CAN_TARGET_${arch})
      add_compiler_rt_static_runtime(clang_rt.${arch} ${arch}
        SOURCES ${${arch}_SOURCES}
        CFLAGS "-std=c99")
      add_dependencies(builtins clang_rt.${arch})
    endif()
  endforeach()
endif()

add_dependencies(compiler-rt builtins)
