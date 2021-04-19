from conans import ConanFile, CMake

class LogppConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = "fmt/7.1.3", "tomlplusplus/2.3.0"
    generators = "cmake"

    options = {
        "build_tests": [True, False],
        "build_benches": [True, False],
        "shared": [True, False]
    }
    default_options = {
        "build_tests": False,
        "build_benches": False,
        "shared": False
    }

    def requirements(self):
        if self.options.build_tests:
            self.requires("gtest/1.10.0")
        if self.options.build_benches:
            self.requires("benchmark/1.5.2")

    def build(self):
        cmake = CMake(self)
        cmake.definitions["BUILD_TESTS"] = self.options.build_tests
        cmake.definitions["BUILD_BENCHES"] = self.options.build_benches
        cmake.configure()
        cmake.build()
        if self.options.build_tests:
            cmake.test()
