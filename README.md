[![CI][ci-shield]][ci-url]
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]


<br />
<p align="center">
  <h3 align="center">logpp</h3>

  <p align="center">
    A modern, fast, structured logging framework in C++
    <br />
    <a href="https://github.com/oktal/logpp"><strong>Explore the docs »</strong></a>
    <br />
    <a href="https://github.com/oktal/logpp/issues">Report Bug</a>
    ·
    <a href="https://github.com/oktal/logpp/issues">Request Feature</a>
  </p>
</p>



<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgements">Acknowledgements</a></li>
  </ol>
</details>


## About The Project

The logging framework you've always dreamed of.<br /><br />
Log events are first recorded from a `Logger`, formatted with a `Formatter` like [logfmt](https://github.com/oktal/logpp/blob/master/include/logpp/format/LogFmtFormatter.h) or [pattern](https://github.com/oktal/logpp/blob/master/include/logpp/format/PatternFormatter.h) and sent to a `Sink` for persistence or visualization like [console](https://github.com/oktal/logpp/blob/master/include/logpp/sinks/ColoredConsole.h), [file](https://github.com/oktal/logpp/blob/master/include/logpp/sinks/file/FileSink.h) and [others](https://github.com/oktal/logpp/tree/master/include/logpp/sinks)

Unlike other well-known logging frameworks, `logpp` has been designed with the idea of enriching log events with information called *structured data*.
The act of adding data to log messages is called [structured logging](https://stackify.com/what-is-structured-logging-and-why-developers-need-it/).

For example, with logpp, a typical structured log event looks like this:

```cpp
logpp::info("Handling http request",
    logpp::field("path", path),
    logpp::field("method", method));
 ```
 
 and will yield the following string rendered into [logfmt format](https://www.brandur.org/logfmt):
 
 `lvl=Info msg="Handling http request" path=/v1/ping method=GET`

At its core, logpp has been designed with a zero-allocation approach, meaning that it will avoid memory allocations for small log events.
Combined with [asynchronocity](https://github.com/oktal/logpp/blob/master/include/logpp/sinks/AsyncSink.h), logpp is a perfect fit for logging in critical code paths.

### Built With

* [conan](https://conan.io/)
* [CMake](https://cmake.org/)

## Getting Started

### Prerequisites

To retrieve logpp external dependencies, make sure to install and configure [conan](https://conan.io) on your local machine.

### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/oktal/logpp.git
   ```
2. Create a build directory
   ```sh
        cd logpp
        mkdir build
    ```
3. Install dependencies
   ```sh
       cd build
       conan install ..
   ```
4. Build the library
   ```sh
       conan build ..
   ```


<!-- USAGE EXAMPLES -->
## Usage

### Namespace

The logpp api is located in the `logpp` namespace

### Basic

```cpp
#include "logpp/logpp.h"

int main(int argc, char* argv[])
{
    logpp::info("This is a log message",
        logpp::field("exe_name", argv[0]));
}
```

```console
2021-04-12 13:28:50 [info] (logpp) This is a log message - exe_name=./bin/sample_BasicLogger
```

logpp provides a default logger that will log all messages into the standard output. logpp `trace`, `debug`, `info`, `warn` and `error` functions will all use
the default logger provided by logpp.

### Manual setup

This example demonstrates how to setup a manual logger to log to a file.

```cpp
#include "logpp/sinks/file/FileSink.h"
#include "logpp/core/Logger.h"

using namespace logpp;

int main()
{
    auto sink = std::make_shared<sink::FileSink>("main.log");
    auto logger = std::make_shared<Logger>("main", LogLevel::Info, sink);

    logger->info("Hello from main");
}
```

```console
> cat main.log 
2021-04-12 13:34:35 [info] (main) Hello from main
```

### Configuration file

Logpp registry can be configured from a [TOML](https://toml.io/en/) configuration file. The following example demonstrates how to do it:

```cpp
#include "logpp/config/TomlConfigurator.h"
#include "logpp/logpp.h"

#include <iostream>

int main(int argc, const char* argv[])
{
    std::string file = "logpp.toml";
    if (argc == 2)
        file = argv[1];

    std::cout << "Configuring logger with " << file << std::endl;
    auto err = logpp::TomlConfigurator::configureFile(file);

    if (err)
    {
        std::cerr << "Error configuring logger: " << *err << std::endl;
        return 0;
    }

    auto logger = logpp::getLogger("main");

    logger->info("This is an informational message",
                 logpp::field("exe_name", argv[0]));
}
```

For a detailed description of logpp configuration format, please refer to [logpp.toml](https://github.com/oktal/logpp/blob/master/examples/logpp.toml)

### Examples

_For more examples, please refer to the [examples](https://github.com/oktal/logpp/examples) folder_


## Roadmap

See the [open issues](https://github.com/othneildrew/Best-README-Template/issues) for a list of proposed features (and known issues).


## Contributing

Contributions are what make the open source community such an amazing place to be learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

Distributed under the MIT License. See `LICENSE` for more information.

[contributors-shield]: https://img.shields.io/github/contributors/oktal/logpp.svg?style=for-the-badge
[contributors-url]: https://github.com/oktal/logpp/graphs/contributors
[ci-shield]: https://img.shields.io/github/workflow/status/oktal/logpp/CI?style=for-the-badge
[ci-url]: https://github.com/oktal/logpp/workflows/CI/badge.svg
[forks-shield]: https://img.shields.io/github/forks/oktal/logpp.svg?style=for-the-badge
[forks-url]: https://github.com/oktal/logpp/network/members
[stars-shield]: https://img.shields.io/github/stars/oktal/logpp.svg?style=for-the-badge
[stars-url]: https://github.com/oktal/logpp/stargazers
[issues-shield]: https://img.shields.io/github/issues/oktal/logpp.svg?style=for-the-badge
[issues-url]: https://github.com/oktal/logpp/issues
[license-shield]: https://img.shields.io/github/license/oktal/logpp.svg?style=for-the-badge
[license-url]: https://github.com/oktal/logpp/blob/master/LICENSE.txt
