# Changelog

## v1.0.7 (May 08, 2025)
* Fixed compatibility with the rpmbuild command of RHEL/Rocky 9.x.
* Fixed build compatibility with recent versions of the Boost library.

## v1.0.5 (Sep 05, 2024)
* Fixed issue that could lead to invalid memory access when shared libs get unloaded during process shutdown.
* Fixed compilation with clang.
* Fixes warnings for .deb package build.
* Thanks to Markus Rothe, Bran Radovanovic, Lawrence Sorrillo for contributions, helpful comments and suggestions.

## v1.0.3 (Jan 01, 2022)
* Don't print O_DIRECT removal log message on EINVAL for files that don't have O_DIRECT flag set.

## v1.0.1 (Dec 30, 2021)
* Initial public release
