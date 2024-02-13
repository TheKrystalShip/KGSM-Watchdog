# log-reader

A simple program written in C that reads live data from `journalctl` and searches for a string pattern match in order to launch events at RabbitMQ.

Made purely because the bash script I had in place that did the exact same thing was using 15-20% CPU and was also falling behind the logs of some very verbose services.

Use at your own risk etc etc.

# Building

Build with `gcc`. There's only one file and it uses system libraries. Default build task for VSCode already set up.
