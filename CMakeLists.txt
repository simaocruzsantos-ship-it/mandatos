# Projeto fxSDK para a Casio fx-CG50
cmake_minimum_required(VERSION 3.15)
project(Mandatos)

include(GenerateG3A)
find_package(Gint 2.1 REQUIRED)

set(SOURCES
  src/main.c
  src/seats.c
)

add_executable(mandatos ${SOURCES})
target_compile_options(mandatos PRIVATE -Wall -Wextra -Os)
target_link_libraries(mandatos Gint::Gint)

generate_g3a(TARGET mandatos OUTPUT "Mandatos.g3a" NAME "Mandatos"
  ICONS assets-cg/icon-uns.png assets-cg/icon-sel.png)
