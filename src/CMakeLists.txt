project(contract)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( contract contract contract.cpp )
target_include_directories( contract PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( contract ${CMAKE_SOURCE_DIR}/../ricardian )

