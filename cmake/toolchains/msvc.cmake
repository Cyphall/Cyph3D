set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Ob3 /DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob3 /DNDEBUG")

add_compile_options($<$<COMPILE_LANGUAGE:C>:/Zc:__STDC__>)
add_compile_options($<$<COMPILE_LANGUAGE:C>:/Zc:__cplusplus>)
add_compile_options($<$<COMPILE_LANGUAGE:C>:/utf-8>)

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/Zc:__STDC__>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/utf-8>)