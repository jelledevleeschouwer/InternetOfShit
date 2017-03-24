# libquickserv (WIP)
IPC Server/Client wrapper around libevquick.

__libquickserv__ serves as a wrapper template for quickly implementing server/client models for IPC-communication on Linux machines. The library is build around [__libevquick__](https://github.com/danielinux/libevquick) that allows for quick event creation with the least amount of system calls. See [__libevquick__](https://github.com/danielinux/libevquick) for more details.

## Usage

__libquickserv__ highly depends on _libevquick_ and thus provides close wrapper-functions for the same utilities that _libevquick_ provides. See [__libevquick__](https://github.com/danielinux/libevquick) for more details. The functions calls are hence the same except for the fact that `libevquick` is replaced by `quickserv`.

For a more detailed explaination of the API, see `libquickserv.h`
