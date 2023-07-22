project(samples)

function(add_shader TARGET SHADER)
  find_program(GLSLC glslc)

  set(current-shader-path ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER})
  set(current-output-path ${CMAKE_BINARY_DIR}/shaders/${SHADER}.spv)

  get_filename_component(current-output-dir ${current-output-path} DIRECTORY)
  file(MAKE_DIRECTORY ${current-output-dir})

  add_custom_command(
        OUTPUT ${current-output-path}
        COMMAND ${GLSLC} -O -MD -MF ${current-output-path}.d -o ${current-output-path} ${current-shader-path}
        DEPENDS ${current-shader-path}
        IMPLICIT_DEPENDS CXX ${current-shader-path}
        VERBATIM)

  set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
  target_sources(${TARGET} PRIVATE ${current-output-path})
endfunction(add_shader)

function(add_sample TARGET)
  add_executable(${TARGET} ${TARGET}.c)
  target_link_libraries(${TARGET} PRIVATE lida_gfx)
endfunction(add_sample)

add_sample(triangle)
add_shader(triangle "triangle.vert")
add_shader(triangle "triangle.frag")

add_sample(cube)
target_link_libraries(cube PRIVATE m)
add_shader(cube "cube.vert")
add_shader(cube "cube.frag")
add_shader(cube "offscreen.vert")
add_shader(cube "offscreen.frag")

add_sample(bloom_teapots)
target_link_libraries(bloom_teapots PRIVATE m)
add_shader(bloom_teapots "model.vert")
add_shader(bloom_teapots "model.frag")
add_shader(bloom_teapots "gamma.frag")
add_shader(bloom_teapots "bloom_read.comp")
add_shader(bloom_teapots "bloom_downsample.comp")
add_shader(bloom_teapots "bloom_upsample.comp")

add_sample(equalizer)
target_link_libraries(equalizer PRIVATE m)
add_shader(equalizer "colored2d.vert")
add_shader(equalizer "colored2d.frag")
add_shader(equalizer "fourier_transform.comp")
