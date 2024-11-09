set(SOURCE_FILE_METAL_SHADER ${CMAKE_CURRENT_SOURCE_DIR}/shader.metal)
set(TMP_FILE_METAL_AIR shader.air)
set(LIB_FILE_METAL_LIB shader.metallib)
# Compile the .metal shader to an .air intermediate file
execute_process(
        COMMAND xcrun -sdk macosx metal -c ${SOURCE_FILE_METAL_SHADER} -o ${TMP_FILE_METAL_AIR}
        RESULT_VARIABLE result_compile
        OUTPUT_VARIABLE output_compile
)
# Check if the first command succeeded
if(NOT result_compile EQUAL "0")
    message(FATAL_ERROR "Compilation failed with result ${result_compile} and output ${output_compile}")
else()
    # Create the .metallib file from the .air file
    execute_process(
            COMMAND xcrun -sdk macosx metallib ${TMP_FILE_METAL_AIR} -o ${LIB_FILE_METAL_LIB}
            RESULT_VARIABLE result_metallib
            OUTPUT_VARIABLE output_metallib
    )

    # Check if the second command succeeded
    if(NOT result_metallib EQUAL "0")
        message(FATAL_ERROR "Metallib creation failed with result ${result_metallib} and output ${output_metallib}")
    else()
        message(STATUS "Create metal shader.metallib file success")
    endif()
endif()