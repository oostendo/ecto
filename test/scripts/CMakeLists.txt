macro(pytest testname)
  add_test(ecto_${testname}
    /usr/bin/env
    PYTHONPATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}:${ecto_SOURCE_DIR}/python:$PYTHONPATH
    ${ARGN}
    )
endmacro()

foreach(file
    test_addergraph
    test_async_bp_objects
    test_async_execution
    test_async_execution_harsh
    test_blackbox
    test_constant
    test_doc
    test_dual_line_plasm
    test_entanglement
    test_exception_in_constructor
    test_If
    test_handles
    test_modules
    test_no_ecto_import
    # test_metrics
    test_parameter_callbacks
    test_passthrough
    test_plasm
    test_process_return_values
    test_python_module
    test_reconnect
    test_redirect
    test_required_io
    test_required_param
    test_random
    test_shared_pass
    test_small_plasms
    test_strands
    test_tendril
    test_tendrils
    test_wrong_param_type
    )
    add_test(ecto_${file} 
        /usr/bin/env
        PYTHONPATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}:${ecto_SOURCE_DIR}/python:$PYTHONPATH
        ${CMAKE_CURRENT_SOURCE_DIR}/${file}.py)

endforeach()

pytest(test_lineplasm0 ${CMAKE_CURRENT_SOURCE_DIR}/test_lineplasm.py 1 1 1 50)
pytest(test_lineplasm1 ${CMAKE_CURRENT_SOURCE_DIR}/test_lineplasm.py 1 5 5 50)
pytest(test_lineplasm2 ${CMAKE_CURRENT_SOURCE_DIR}/test_lineplasm.py 10 10 10 50)
pytest(test_lineplasm3 ${CMAKE_CURRENT_SOURCE_DIR}/test_lineplasm.py 15 25 35 50)
