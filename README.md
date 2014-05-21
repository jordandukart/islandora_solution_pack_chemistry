# Chemistry Solution Pack [![Build Status](https://travis-ci.org/discoverygarden/islandora_solution_pack_chemistry.png?branch=7.x)](https://travis-ci.org/discoverygarden/islandora_solution_pack_chemistry)

## Introduction

Islandora Chemistry Solution Pack Module

Chemistry Solution Pack for Islandora
Loads all required Fedora Objects, and creates an empty collection object to accept ingested files related to chemistry


All bugs, feature requests and improvement suggestions are tracked at the [DuraSpace JIRA](https://jira.duraspace.org/browse/ISLANDORA).

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora) - islandora core functionality
* [Tuque](https://github.com/islandora/tuque) - The wrapper Islandora uses to communicate with Fedora Commons
* [OpenBabel](http://openbabel.org) - Required for basic functionality to work. OpenBabel converts the chemical structure files into a common format, creates images from them and extracts useful chemical data.
* [ChemDoodleWeb](http://web.chemdoodle.com/) - is used as a viewer for the object and allows a 3D representation of the molecule to be displayed. It is also used as a molecule editor so that structures can be used for searching.
* [Indigo python API](http://ggasoftware.com/opensource/indigo) - is required, along with a couple of python scripts, to create and search for molecular fingerprints. These allow for substructures to be searched and retrieved. Upon ingest the fingerprint of the molecule is stored and so this package has to be present at this point if you want to use substructure searching on all of the molecules. Indigo requires at least python version 2.6.6 to run. If your OS doesn't have at least this version available from your package manager then you can download a standalone version of python from [Activestate](https://www.activestate.com/activepython).
* [Checkmol](http://merian.pch.univie.ac.at/~nhaider/cheminf/cmmm.html) - Required to identify functional groups in the molecule and allow searching based on that.

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

## Configuration

Configure each module/library as outlined in there respective installation instructions.

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors

Current maintainers:

* [discoverygarden inc.](https://github.com/discoverygarden)

Past maintainers:

* [Richard Wincewicz](https://github.com/rwincewicz)
* [Robertson Library](https://github.com/roblib)

## Development

If you would like to contribute to this module, please check out our helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
