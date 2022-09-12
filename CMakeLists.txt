cmake_minimum_required(VERSION 3.5)

idf_component_register(
                       INCLUDE_DIRS "src"
                       SRC_DIRS "src"
                       REQUIRES arduino
                       )
project(ezTime)
