{
  "targets": [
    {
      "target_name": "node-audio-stem",
      "sources": [
        "addon.cpp"
      ],
      "include_dirs": [
        "../src",
        "../vendor/eigen",
        "../vendor/eigen/bench",
        "../vendor/libnyquist/include",
        "../vendor/libnyquist/third_party/include",
        "../vendor/googletest/googletest/include",
        "./node_modules/node-addon-api"
      ],
      "libraries": [
        "<(module_root_dir)/../build/libdemucs.cpp.lib.a",
        "<(module_root_dir)/../build/lib/libgtest_main.a",
        "<(module_root_dir)/../build/vendor/libnyquist/lib/liblibnyquist.a",
        "-lopenblas"
      ],
      "cflags!": [
        "-fexceptions"
      ],
      "cflags_cc": [
        "-DEIGEN_USE_BLAS"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS",
        "EIGEN_USE_BLAS"
      ]
    }
  ]
}