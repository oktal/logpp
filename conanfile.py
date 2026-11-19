from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain

required_conan_version = ">=2.1"

class LogppConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = "fmt/12.2.0", "tomlplusplus/2.5.0"

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

    exports_sources = "*"

    def build_requirements(self):
        if self.options.build_tests:
            self.test_requires("gtest/1.10.0")
        if self.options.build_benches:
            self.test_requires("benchmark/1.5.6")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        toolchain = CMakeToolchain(self)
        toolchain.variables["LOGPP_BUILD_TESTS"] = bool(self.options.build_tests)
        toolchain.variables["LOGPP_BUILD_BENCHES"] = bool(self.options.build_benches)
        toolchain.variables["LOGPP_SHARED"] = bool(self.options.shared)
        toolchain.variables["BUILD_SHARED_LIBS"] = bool(self.options.shared)
        toolchain.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if self.options.build_tests:
            cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

