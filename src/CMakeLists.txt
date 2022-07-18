include_directories(lib/include)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.74.0 REQUIRED)

set(BASKET_PRICER_LIB_SOURCE
        lib/basketpricer/Basket.cpp
        lib/basketpricer/BasketPricer.cpp
        lib/marketdata/TickEvent.cpp
        lib/simulation/RandomDistributionGenerator.cpp
        lib/simulation/TickDataGenerator.cpp
        lib/util/CSVReader.cpp)

add_library(basket_simulation_lib ${BASKET_PRICER_LIB_SOURCE})

set(SIM_BASKET_PRICER_SOURCE
        app/SimulateBasketPricer.cpp)

add_executable(SimulateBasketPricer ${SIM_BASKET_PRICER_SOURCE})
target_link_libraries(SimulateBasketPricer basket_simulation_lib)

set(SHAPE_VISITOR_SOURCE
        visitor/ShapeVisitor.cpp)
add_executable(ShapeVisitor ${SHAPE_VISITOR_SOURCE})

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(basket_simulation_lib ${Boost_LIBRARIES})
    target_link_libraries(SimulateBasketPricer ${Boost_LIBRARIES})
endif ()

set_target_properties(SimulateBasketPricer
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )

set_target_properties(ShapeVisitor
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )