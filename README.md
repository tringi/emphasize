# emphasize
Helper functions and classes I wrote for my various projects. Some happen to be trivial, others are more complicated. There are often dependencies between units and there will probably be bugs. Documentation for each feature is provided right in the appropriate header file (or not). Not everything is feature-complete so feel free to contact me if you wish to use something and don't figure stuff out.

## Windows
Functions simplifying or extending Windows API. Function and file name usually reveal its purpose.

## Resources
Nicer access to various PE .rsrc (resources) data. Functions usually provide LANGID choice which Windows API functions generally don't. Fallback search for a resource of appropriate language (if requested one isn't present) tries hard to find the best matching (see Resources_Language.hpp)

## Emphasize
Utilities written exclusively for abandoned Emphasize project but found generally useful.

## Emphasize
A few Win32 controls that don't store data, everything is queried by callbacks.

## License
This project is a free software available under the zlib license.
