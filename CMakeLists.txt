cmake_minimum_required(VERSION 3.13.4)

project(SlicerCollaboration)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "https://www.slicer.org/wiki/Documentation/Nightly/Extensions/SlicerCollaboration")
set(EXTENSION_CATEGORY "SlicerCollaboration")
set(EXTENSION_CONTRIBUTORS "Csaba Pinter (Ebatinca S.L.), David Garcia-Mato (Ebatinca S.L.)")
set(EXTENSION_DESCRIPTION "Extension facilitating real-time collaboration between two 3D Slicer instances")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/SlicerCollaboration.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/SlicerCollaboration/Screenshots/1.png")
set(EXTENSION_DEPENDS "NA") # Specified as a list or "NA" if no dependencies

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(Collaboration)
add_subdirectory(Chat)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
