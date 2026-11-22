include(${CMAKE_BINARY_DIR}/_deps/xcmake_tools/Common.cmake)

# Fetch dependencies
FetchAndPopulate("https://github.com/LIONant-depot/xbits.git" "main")
FetchAndPopulate("https://github.com/LIONant-depot/xserializer.git" "main")
FetchAndPopulate("https://github.com/LIONant-depot/xresource_pipeline_v2.git" "main")
FetchAndPopulate("https://github.com/LIONant-depot/xstrtool.git" "main" )
FetchAndPopulate("https://github.com/LIONant-depot/xscheduler.git" "main")
FetchAndPopulate("https://github.com/LIONant-depot/xAnimation.git" "main")


DefineInterfaceComponent(xlocomotion_compiler_ "dependencies"
  "source/xlocomotion.h"
  "source/xlocomotion_ref.h"
  "source/xlocomotion_descriptor.h"
  "source/compiler/xlocomotion_compiler.cpp"
  "source/compiler/xlocomotion_compiler.h"
)
