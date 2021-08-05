<a name="v0.1.3"></a>
## [v0.1.3](https://github.com/oktal/logpp/compare/v0.1.2...v0.1.3) (2021-08-05)

### Chore
- Bump project version ([a3a2d66](https://github.com/oktal/logpp/commit/a3a2d665205fe99c233cb8cdad651abbb6aa0beb))
- Update CHANGELOG ([6642eef](https://github.com/oktal/logpp/commit/6642eef58ed5535139f2c46350fafc610bbb4beb))

### Features
- **core:** Add a way to register a default logger factory function ([4990bb6](https://github.com/oktal/logpp/commit/4990bb61f4cbcb2bb9fece5d1167dd8a4dc1e45c))


<a name="v0.1.2"></a>
## [v0.1.2](https://github.com/oktal/logpp/compare/v0.1.1...v0.1.2) (2021-05-27)

### Bug Fixes
- **core:** add LLVM libFuzzer-based fuzz tests and fix ASAN-detected memory issues ([f385876](https://github.com/oktal/logpp/commit/f385876e39e2642fdc1372ae201e70c0d3d2614f))
- **sinks:** Add missing localtime conversion when rolling with a `RoundInterval` ([73c8f4b](https://github.com/oktal/logpp/commit/73c8f4b8ab8d6f0ab18a9e4aafd3c6548bbf5ca5))
- **sinks:** Correctly parse `FormatSink` formatter options ([91db685](https://github.com/oktal/logpp/commit/91db68574a9dddf97d21623c65506b3963b0968a))
- **sinks:** Update `rolling_ofstream` `RollEvery` logic when rolling with a round interval. ([91a4e3d](https://github.com/oktal/logpp/commit/91a4e3d02e423700256541a0158df92a9314fe0a))
- Correctly parse time durations and allow negative durations ([8b8f3ec](https://github.com/oktal/logpp/commit/8b8f3eca8320b6292f02f333721281ea1ced5118))
- Update stop sequence order of `FileWatcher` to avoid deadlocking. ([fa0f3d5](https://github.com/oktal/logpp/commit/fa0f3d5e2459b7f749559f99b7bbe47f65ae2aad))

### Chore
- **CMake:** Use `set_property` instead of `target_*` functions in FindFilesystem to support older versions of CMake. ([d17babe](https://github.com/oktal/logpp/commit/d17babecc6897b2c338acb7240627f9062639daa))
- **config:** Simplify `TomlConfigurator` logger missing fields filling ([d98d695](https://github.com/oktal/logpp/commit/d98d6955a26ed230be0df30c8ff4d673d352a565))
- **core:** Fix implicit type conversion warning ([10e307e](https://github.com/oktal/logpp/commit/10e307e68f5a8c9f3cf6811bf2318b45ff5e1087))
- **core:** Move `SimpleTypedBlockingQueue` to its own header file. ([160d2c3](https://github.com/oktal/logpp/commit/160d2c31c5cb0e21e916655a1c345a9a2ca26fdd))
- **queue:** disable structure alignment padding warning on msvc ([17f6305](https://github.com/oktal/logpp/commit/17f6305f09ae3bb94c91a06795e87b0dc277c61b))

### Features
- **config:** Add time shifting configuration option to timestamp archive policy ([59497c3](https://github.com/oktal/logpp/commit/59497c3d3ba3df93b1f6ec465bd58e02d8e0ecf3))
- **config:** Make `level` and `sinks` configuration parameters for logger optional ([b1af75d](https://github.com/oktal/logpp/commit/b1af75dd999430da6a1a1638221f0baa4ad0e594))
- **core:** Add `Off` to `LogLevel` that can be used to disable a logger ([f96bbd2](https://github.com/oktal/logpp/commit/f96bbd21c5a38a9dd91a9a1373d90217ea3414c4))
- **core:** Keep track of loggers created through the default factory to allow out-of-order initialization of registry. ([7dc6df3](https://github.com/oktal/logpp/commit/7dc6df33f2ac843863af7e555644c668fba50b70))
- **format:** Add a format specifier to convert time to local time. ([9cc4a23](https://github.com/oktal/logpp/commit/9cc4a2397593f78a106128f7f2edceede5420d30)), related to [#21](https://github.com/oktal/logpp/issues/21)


<a name="v0.1.1"></a>
## [v0.1.1](https://github.com/oktal/logpp/compare/v0.1.0...v0.1.1) (2021-04-26)

### Bug Fixes
- **sinks:** Correctly call `onAfterOpened` for `RollingFileSink` ([76f22af](https://github.com/oktal/logpp/commit/76f22af228c07435abac5d8904bf9c686cc1f6f5))
- **sinks:** Do not spuriously throw configuration error for `RollingFileSink` ([f8c5944](https://github.com/oktal/logpp/commit/f8c5944416528fe6e5980585a8a3319348b52066))
- Fix possible invalid iterator usage in string_utils::iequals ([11c444a](https://github.com/oktal/logpp/commit/11c444a2859274c02f4e534697b72db137d26af9))

### Chore
- **CMake:** Add Filesystem script to correctly link with stdc++fs if needed ([211017f](https://github.com/oktal/logpp/commit/211017f0c9fddc1c3eda9810c03803c284630afa))
- **CMake:** Print message when copying file next to binary ([2a231e1](https://github.com/oktal/logpp/commit/2a231e17171a78e0e41c1eaafd76b3b591d5a554))
- Bump patch version number to 0.1.1 ([d56f21e](https://github.com/oktal/logpp/commit/d56f21e05bf4768b5acb80653b5de774d59aeebe))

### Enhancement
- **CMake:** Link back with stdc++fs for gcc ([8d72114](https://github.com/oktal/logpp/commit/8d72114582ac6aef0a08a3e9a8a75b7dbeada334))
- **CMake:** Only generate install target by default when logpp is not included as a subdirectory ([89d703e](https://github.com/oktal/logpp/commit/89d703eebf00776d11cdb445c602c18a31b995dd))

### Features
- **sinks:** Add `ColoredConsole` TOML theme configuration ([cc523b5](https://github.com/oktal/logpp/commit/cc523b5cca85e745b0887bf7bc671d8d7fcb60f4))
- **sinks:** Add `Console` sink ([9ed96a6](https://github.com/oktal/logpp/commit/9ed96a60bb2dfbf92a30bba96b51bd2c283a94b7)), related to [#19](https://github.com/oktal/logpp/issues/19)

### Style
- format code ([0884115](https://github.com/oktal/logpp/commit/08841155fc9ae06fef9d4f0c289b98838962888a))


<a name="v0.1.0"></a>
## [v0.1.0](https://github.com/oktal/logpp/compare/ce1a7a609513f7eccf09e10dab91f8b5e5f7220f...v0.1.0) (2021-04-19)

### Bug Fixes
- **core:** Correctly format text with parenthesis ([716ae63](https://github.com/oktal/logpp/commit/716ae63fb9e4266c2859e97dc4aab56f3882edd7))
- **format:** Fix failing LogFmtFormatterTests with gcc ([d19be4a](https://github.com/oktal/logpp/commit/d19be4af293e0e28d71a652ed24b7bfd710b4a7c))

### Chore
- **build:** Remove build directory output setup from CMakeLists and let conan do the right setup ([6100f5c](https://github.com/oktal/logpp/commit/6100f5c954d11a39219bd72ae5489d078c839e8d))
- **core:** Fix clang build ([d3bf2fe](https://github.com/oktal/logpp/commit/d3bf2fe06cc00efe102790d427f1186dcc81c018))
- **core:** Get rid of LoggerFactory ([99e852f](https://github.com/oktal/logpp/commit/99e852f95e2e13dd2067f31c523e9f825836d7fa))
- **core:** make LoggerRegistry thread-safe ([b6af4cb](https://github.com/oktal/logpp/commit/b6af4cb01d25d6cf4ad8b12ba980b0bebeccd571))
- **core:** Renaming ([0c337d6](https://github.com/oktal/logpp/commit/0c337d6e4808a0c9562e4ab7c7332ccc406428b6))
- **format:** Cleanup FieldsFormatter ([73feafb](https://github.com/oktal/logpp/commit/73feafbed156180bfefed6df9e1700087b17b866))
- **sinks:** Fix initialization order warning ([ed03f16](https://github.com/oktal/logpp/commit/ed03f1679953747edb2fbe23b17a465f8d3e1b55))
- **workflows:** Enable CI on Windows ([0094304](https://github.com/oktal/logpp/commit/0094304844727527111582389dedd99aba538a71))
- Add .clang-format ([05d9a02](https://github.com/oktal/logpp/commit/05d9a021cf966dfb34ef7bb3e6ca7a484b1c5ade))
- Add a format shell script and make target ([11b4423](https://github.com/oktal/logpp/commit/11b4423ff2aa169fc04ece4da68c0409d5b3eb0e))
- Add Features section in README.md ([428b729](https://github.com/oktal/logpp/commit/428b729fbe3e5bc600580a7c2982d28984eaa0b4))
- Code format ([ddbe438](https://github.com/oktal/logpp/commit/ddbe438ad59704fc90e47197230c3eabbb31ec1a))
- Code style fixes ([314ab62](https://github.com/oktal/logpp/commit/314ab624458b0c051473d6bceaf7da4503bf8f59))
- Fix CMakeLists.txt ([2061e08](https://github.com/oktal/logpp/commit/2061e08c3561c8c45776901aa1208508c967485d))
- Fix windows build ([f81d23b](https://github.com/oktal/logpp/commit/f81d23b052da55efc1f3a732cab9a9113b456879))
- Format ([0492924](https://github.com/oktal/logpp/commit/04929242373958c2a106ad4d88bfac79f57d4559))
- Provide a way to specify dependencies other than using conan ([0729795](https://github.com/oktal/logpp/commit/072979503c0e2022f1ed500cc0cd4163bf778d82))
- README ([7941e72](https://github.com/oktal/logpp/commit/7941e72c0801b976a1f9be7470b57223ea0ae8c1))

### Enhancement
- **CMake:** Add CPack ([1ae315a](https://github.com/oktal/logpp/commit/1ae315a521513a4bec5f7ea67e7b0353e062b3ed))
- **CMake:** Add install target ([54a181f](https://github.com/oktal/logpp/commit/54a181f4d07ddef139d4fca95978219af2a73e45))
- **config:** Add file parsing to TomlConfigurator ([b18ceb6](https://github.com/oktal/logpp/commit/b18ceb6f7baa86eadf635588dd2048f3bd844c50))
- **core:** Add a registerFlag<F> function to PatternFormatter to register custom flag formatter. ([f67fcbf](https://github.com/oktal/logpp/commit/f67fcbf8edbe472cb13f2e4fb7fc661267e1d53d)), related to [#4](https://github.com/oktal/logpp/issues/4)
- **core:** Add free log functions ([2f8b10d](https://github.com/oktal/logpp/commit/2f8b10df70799e6771046514ca273ab013d64e21))
- **core:** Add LOGPP_* logging macros and source location ([fa2b4ac](https://github.com/oktal/logpp/commit/fa2b4acb02ca721b02d6e13b7fc6d2f49a598650))
- **core:** Add thread id to EventLogBuffer ([73a4d69](https://github.com/oktal/logpp/commit/73a4d6947463b0139599a4c7c5dd213148b427d6))
- **core:** Correctly take LogLevel into account when logging messages ([fd3ede8](https://github.com/oktal/logpp/commit/fd3ede8fbb64a726f10a426e3e330c21b6c74252))
- **core:** Handle boolean fields ([da4da85](https://github.com/oktal/logpp/commit/da4da858485fbac6987ec39c9e1ab4337a4f9416)), related to [#17](https://github.com/oktal/logpp/issues/17)
- **core:** logpp::data() now becomes logpp::field and rename related types ([9b8b7af](https://github.com/oktal/logpp/commit/9b8b7aff1d2ff19ca969320d7dc69fefac855cc8))
- **core:** StringLiteralOffset now really holds an offset to the buffer ([08e26be](https://github.com/oktal/logpp/commit/08e26be2bdc63300985ff8ae6f93d40f2d73da17))
- **format:** Add pattern to LogFmtFormatter ([d28c66f](https://github.com/oktal/logpp/commit/d28c66f4d86345eabfb3564e6d02fe20ee351409))
- **format:** Add thread id to PatternFormatter and LogFormatter ([35223b2](https://github.com/oktal/logpp/commit/35223b2c9c00dd175bf1e32d6ed7cb977b33351b))
- **format:** Make LogFmt a Formatter, not a Sink ([3abe380](https://github.com/oktal/logpp/commit/3abe380c32830c35b1112d6b26e60b6441c315e0))
- **format:** Use date library to convert timepoint to date and time instead of tm type ([0bd7349](https://github.com/oktal/logpp/commit/0bd73499397c2ba3db7321c90466b27565ce6c8b))
- **platform:** Fix build under msvc ([4832a69](https://github.com/oktal/logpp/commit/4832a69e6b40d6ec119e8950a02d45b6b57ff5f1))
- **sink:** Add a way to override FileSink ([9815052](https://github.com/oktal/logpp/commit/9815052b267f4896067c9112aac0380bf0ab6572))
- **sink:** Add options to FileSink ([949b517](https://github.com/oktal/logpp/commit/949b517edfcc8fe81601fdbfbb9a6780de7bb105))
- **sink:** Correctly activate Sink options and update RollingFileSink ([ee4d4d9](https://github.com/oktal/logpp/commit/ee4d4d9b0d0b6d1c1be400bc0d5ead2146d6ec6d))
- **sink:** Migrate sinks configuration to `ConfigurationError` exception class to give more context to the user ([2771966](https://github.com/oktal/logpp/commit/277196677a9fc0291ab8bf2a03a44527db96fdeb))
- **sink:** Re-organize file sink structure ([6183b85](https://github.com/oktal/logpp/commit/6183b8548bddb0a3e95f7e458e6b9bf4fed9e951))
- **sink:** Rename FileSink name to File ([86885c2](https://github.com/oktal/logpp/commit/86885c2db6b766c74aacdc7f5a20947aff03bfce))
- **sinks:** Add rolling_ofstream implementation based on std::basic_stream interface ([653e6f5](https://github.com/oktal/logpp/commit/653e6f5c52b9374e30dec5735339ef164c6295b7))
- **sinks:** Correctly mark RollingOfStream function as override and call mode() in temporary_filebuf ([87e773b](https://github.com/oktal/logpp/commit/87e773b93f315f965d5bdc146ffdff76ad916d55))
- **sinks:** Make File destructor virtual ([fac43cb](https://github.com/oktal/logpp/commit/fac43cb5121e6973981e629340eaccb4626798f2))
- **sinks:** Replace RollingStrategy and ArchiveStrategy by rolling_ofstream in RollingFileSink ([3474324](https://github.com/oktal/logpp/commit/34743247a257ed8700217618affb3d4c497fc929))
- **sinks:** Use `rang` library to format color in ColoredConsole ([4918c5f](https://github.com/oktal/logpp/commit/4918c5f2af1b898f6d3ed33a7df69484a4777aa9))
- Fix clang build ([522dfff](https://github.com/oktal/logpp/commit/522dfff9460faa4d89c9bdfe69487dc8bbeb8312))

### Features
- **api:** Remove evented-like logging from API ([0e83f90](https://github.com/oktal/logpp/commit/0e83f90d357d6bc7db92a94739eec27a66c6b05d))
- **config:** Add and default to a noop version of FileWatcher when implementation is not available on current platform ([c9482ca](https://github.com/oktal/logpp/commit/c9482ca92813a5c1abbbb07a62f8cd16842e59c7))
- **config:** Add FileWatcher implementation for Windows ([ad86ca5](https://github.com/oktal/logpp/commit/ad86ca5d689d73d1ed8c07f6c1e72566f2124aa9))
- **config:** Add TomlConfigurator to configure loggers though TOML ([5cc5d80](https://github.com/oktal/logpp/commit/5cc5d80932891b32a5b8d1b7702d437d4ca3d7e2))
- **config:** Default logger can now be configured from TOML ([8923950](https://github.com/oktal/logpp/commit/89239508d830e259c9fd9e8dfb0b229fca9b5aa8))
- **core:** Add a registry ([dbf759e](https://github.com/oktal/logpp/commit/dbf759e5310b024b7b1c05777f9c9e23193608f2))
- **core:** Add structured logging ([bd65e02](https://github.com/oktal/logpp/commit/bd65e02dc54b2e8d0500e8f23f35a329372941e0))
- **core:** Add support for fields that provide an ostream operator<< ([4cff806](https://github.com/oktal/logpp/commit/4cff806136712c83acf1784decbf84ddee552ddb))
- **core:** Added a format-like API for log message ([27e03e7](https://github.com/oktal/logpp/commit/27e03e7b020b06f3f9f412f57c7598dbfebe331b))
- **core:** Rework structured data visiting inside LogBuffer ([83b531c](https://github.com/oktal/logpp/commit/83b531c104e33cb3aff09d516d7219d0a84b0ea0))
- **format:** Add '%i' milliseconds pattern formatter flag and '%u' microseconds pattern formatter flag ([a0b2197](https://github.com/oktal/logpp/commit/a0b2197b456554cb6fe962edec2cdd4a4490ce64))
- **format:** Add a Formatter interface and PatternFormatter implementation ([4f516b7](https://github.com/oktal/logpp/commit/4f516b7c9a930b480a0478731e2c003a24b49c9d))
- **format:** Support structured fields formatting in PatternFormatter ([7b326a5](https://github.com/oktal/logpp/commit/7b326a5b324e95982ed324a651b5074faf7644a8))
- **sink:** Add logger name ([5466d88](https://github.com/oktal/logpp/commit/5466d8808b5b6aa30ead86bfcec11f73b41a6608))
- **sink:** Add starting implementation of RollingFileSink ([1ac6a79](https://github.com/oktal/logpp/commit/1ac6a7992f3ffe68e8014727cdb154188f3b5958))
- **sink:** Colorize level in ColoredOutputConsole ([9e03fa8](https://github.com/oktal/logpp/commit/9e03fa84655f3b0fba0f72bffc99ae8cc2517d4a))
- **sink:** Making progress on RollingFileSInk ([bf1f7fb](https://github.com/oktal/logpp/commit/bf1f7fbc6910387a8fb6048d417f50c6af6a589b))
- **sink:** Revamp configuration options and put some more work into RollingFileSink ([0413d16](https://github.com/oktal/logpp/commit/0413d16965267393ae139514c52b4f86ab6b2846))
- **sinks:** Add a basic FileSink ([888f41f](https://github.com/oktal/logpp/commit/888f41f034c3d5747bb2f6bc0c31f762b0c2ad1f))
- **sinks:** Add configuration options to AsyncSink ([0b57283](https://github.com/oktal/logpp/commit/0b57283afa242026e46e99cd3de6a3f033cc83e9))
- **sinks:** Add level option to TOML sink configuration. ([4782a60](https://github.com/oktal/logpp/commit/4782a60fbe9fb9dac63aff03aa1775aed8d970ce)), related to [#8](https://github.com/oktal/logpp/issues/8)
- **sinks:** Refactored ColoredOutputConsole to ColoredConsole and added ColoredErrorConsole ([416c735](https://github.com/oktal/logpp/commit/416c73513cdea0e69e7ee53380d4fa09a8eda49c))


