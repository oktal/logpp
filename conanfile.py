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

    exports_sources = "*"

    _cmake = False

    def requirements(self):
        if self.options.build_tests:
            self.requires("gtest/1.10.0")
        if self.options.build_benches:
            self.requires("benchmark/1.5.2")

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()
        if self.options.build_tests:
            cmake.test()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def _configure_cmake(self):
        if self._cmake:
            return self._cmake

        self._cmake = CMake(self)
        self._cmake.definitions["LOGPP_BUILD_TESTS"] = self.options.build_tests
        self._cmake.definitions["LOGPP_BUILD_BENCHES"] = self.options.build_benches
        self._cmake.configure()
        return self._cmake

