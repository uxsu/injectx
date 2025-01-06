add_injectx_module(json)

find_package(fmt CONFIG REQUIRED)

target_sources(${injectx_module_target}
  PRIVATE
    include/injectx/json/parser/token.hpp
)

target_link_libraries(${injectx_module_target}
  PUBLIC
    fmt::fmt
)

set_target_properties(${injectx_module_target}
  PROPERTIES
    LINKER_LANGUAGE
    CXX
)
