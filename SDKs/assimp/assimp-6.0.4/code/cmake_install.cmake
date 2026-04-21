# Install script for directory: C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Assimp")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libassimp6.0.4-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/lib/Debug/assimp-vc143-mtd.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/lib/Release/assimp-vc143-mt.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/lib/MinSizeRel/assimp-vc143-mt.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/lib/RelWithDebInfo/assimp-vc143-mt.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libassimp6.0.4" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/bin/Debug/assimp-vc143-mtd.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/bin/Release/assimp-vc143-mt.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/bin/MinSizeRel/assimp-vc143-mt.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/bin/RelWithDebInfo/assimp-vc143-mt.dll")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "assimp-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp" TYPE FILE FILES
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/anim.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/aabb.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/ai_assert.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/camera.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/color4.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/color4.inl"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/config.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/ColladaMetaData.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/commonMetaData.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/defs.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/cfileio.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/light.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/material.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/material.inl"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/matrix3x3.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/matrix3x3.inl"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/matrix4x4.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/matrix4x4.inl"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/mesh.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/ObjMaterial.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/pbrmaterial.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/GltfMaterial.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/postprocess.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/quaternion.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/quaternion.inl"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/scene.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/metadata.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/texture.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/types.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/vector2.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/vector2.inl"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/vector3.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/vector3.inl"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/version.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/cimport.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/AssertHandler.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/importerdesc.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Importer.hpp"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/DefaultLogger.hpp"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/ProgressHandler.hpp"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/IOStream.hpp"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/IOSystem.hpp"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Logger.hpp"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/LogStream.hpp"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/NullLogger.hpp"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/cexport.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Exporter.hpp"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/DefaultIOStream.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/DefaultIOSystem.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/ZipArchiveIOSystem.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/SceneCombiner.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/fast_atof.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/qnan.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/BaseImporter.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Hash.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/MemoryIOWrapper.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/ParsingUtils.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/StreamReader.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/StreamWriter.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/StringComparison.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/StringUtils.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/SGSpatialSort.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/GenericProperty.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/SpatialSort.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/SkeletonMeshBuilder.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/SmallVector.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/SmoothingGroups.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/SmoothingGroups.inl"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/StandardShapes.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/RemoveComments.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Subdivision.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Vertex.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/LineSplitter.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/TinyFormatter.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Profiler.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/LogAux.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Bitmap.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/XMLTools.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/IOStreamBuffer.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/CreateAnimMesh.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/XmlParser.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/BlobIOSystem.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/MathFunctions.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Exceptional.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/ByteSwapper.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Base64.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "assimp-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp/Compiler" TYPE FILE FILES
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Compiler/pushpack1.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Compiler/poppack1.h"
    "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/../include/assimp/Compiler/pstdint.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/bin/Debug/assimp-vc143-mtd.pdb")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/bin/Release/assimp-vc143-mt.pdb")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/bin/MinSizeRel/assimp-vc143-mt.pdb")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/bin/RelWithDebInfo/assimp-vc143-mt.pdb")
  endif()
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/vicen/source/repos/vicenteconejerosdelacruz/Engine/SDKs/assimp/assimp-6.0.4/code/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
