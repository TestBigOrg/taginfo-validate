## taginfo-validate

[![Continuous Integration](https://travis-ci.org/mapbox/taginfo-validate.svg?branch=master)](https://travis-ci.org/mapbox/taginfo-validate)

- [Taginfo schema](https://wiki.openstreetmap.org/wiki/Taginfo/Projects)

## Building

    pushd third_party && ./build.sh && popd
    ./build.sh

With [Nix](https://nixos.org/nix/):

    nix-shell --pure --run 'pushd third_party && ./build.sh && popd'
    nix-shell --pure --run './build.sh'

## License

Copyright © 2016 Mapbox

Distributed under the MIT License (MIT).
