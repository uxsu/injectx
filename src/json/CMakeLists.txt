add_injectx_module(json)

find_package(fmt CONFIG REQUIRED)

target_sources(${injectx_module_target}
  PRIVATE
    include/injectx/json/parser/token.hpp
    include/injectx/json/parser/utility.hpp
    include/injectx/json/parser/tokenizer.hpp
    include/injectx/json/parser/stringparser.hpp
)

target_link_libraries(${injectx_module_target}
  PUBLIC
    fmt::fmt
    injectx::args
    injectx::stdext
)

set_target_properties(${injectx_module_target}
  PROPERTIES
    LINKER_LANGUAGE
    CXX
)
