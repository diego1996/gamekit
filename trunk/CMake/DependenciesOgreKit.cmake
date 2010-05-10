#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------
include(MacroLogFeature)


#######################################################################
# Core dependencies
#######################################################################

# Find X11
if (UNIX) 
    ## TEST THIS
    find_package(X11)
    macro_log_feature(X11_FOUND "X11" "X Window system" "http://www.x.org" TRUE "" "")
    macro_log_feature(X11_Xt_FOUND "Xt" "X Toolkit" "http://www.x.org" TRUE "" "")
    find_library(XAW_LIBRARY NAMES Xaw Xaw7 PATHS ${DEP_LIB_SEARCH_DIR} ${X11_LIB_SEARCH_PATH})
    macro_log_feature(XAW_LIBRARY "Xaw" "X11 Athena widget set" "http://www.x.org" TRUE "" "")
    mark_as_advanced(XAW_LIBRARY)
endif ()


# Find OpenGL
find_package(OpenGL)
macro_log_feature(OPENGL_FOUND "OpenGL" "Support for the OpenGL render system" "http://www.opengl.org/" FALSE "" "")

# Find DirectX
if(WIN32)
	find_package(DirectX)
	macro_log_feature(DirectX_FOUND "DirectX" "Support for the DirectX render system" "http://msdn.microsoft.com/en-us/directx/" FALSE "" "")
endif()

# Find Cg
find_package(Cg)
macro_log_feature(Cg_FOUND "cg" "C for graphics shader language" "http://developer.nvidia.com/object/cg_toolkit.html" FALSE "" "")

#######################################################################
# Apple-specific
#######################################################################
if (APPLE)
    find_package(Carbon)
    macro_log_feature(Carbon_FOUND "Carbon" "Carbon" "http://developer.apple.com/mac" TRUE "" "")

    find_package(Cocoa)
    macro_log_feature(Cocoa_FOUND "Cocoa" "Cocoa" "http://developer.apple.com/mac" TRUE "" "")

    find_package(IOKit)
    macro_log_feature(IOKit_FOUND "IOKit" "IOKit HID framework needed by the samples" "http://developer.apple.com/mac" FALSE "" "")
endif(APPLE)

# Display results, terminate if anything required is missing
MACRO_DISPLAY_FEATURE_LOG()

# Add library and include paths from the dependencies
include_directories(
  ${OPENGL_INCLUDE_DIRS}
  ${Cg_INCLUDE_DIRS}
  ${X11_INCLUDE_DIR}
  ${DirectX_INCLUDE_DIRS}
  ${Carbon_INCLUDE_DIRS}
  ${Cocoa_INCLUDE_DIRS}
)

link_directories(
  ${OPENGL_LIBRARY_DIRS}
  ${Cg_LIBRARY_DIRS}
  ${X11_LIBRARY_DIRS}
  ${DirectX_LIBRARY_DIRS}
)