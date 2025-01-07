add_injectx_module(args)

target_sources(${injectx_module_target}
  PRIVATE
    include/injectx/args/args.hpp
)

set_target_properties(${injectx_module_target}
  PROPERTIES
    LINKER_LANGUAGE
    CXX
)
