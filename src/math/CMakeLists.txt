# SPDX-FileCopyrightText: Copyright 2024 Mikhail Svetkin
# SPDX-License-Identifier: MIT

add_injectx_module(math)

target_sources(${injectx_module_target}
  PRIVATE
    include/injectx/math/bigint.hpp
)

target_link_libraries(${injectx_module_target}
  PUBLIC
    injectx::stdext
    injectx::args
)

set_target_properties(${injectx_module_target}
  PROPERTIES
    LINKER_LANGUAGE
    CXX
)
