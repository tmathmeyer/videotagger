RSVTM
=====
##Really Simple Video Tag Manager
A filesystem based video tag manager (for linux)



A "Media Directory" is a directory with a .rsvtm file

A "Media Directory" is a directory with a .tags subdirectory
  - the .tags subdirectory contains one directory for each tag in the system
  - each tag directory in the .tags subdirectory contains symlinks to media files
  - There will be at least one tag called [Tracked] which is the definitive tag for all files being tracked.

A "Media Directory" is a directory with a .applied subdirectory
  - Each filename in .tags/Tracked will have a directory in .applied/
  - each directory in .applied will contain symlinks to each tag directory that the file has

example directory structure:
```
$ tree -la
.
├── .applied
│   └── Clouds.jpg
│       └── Tracked -> ../../.tags/Tracked/
│           └── Clouds.jpg -> ../../Clouds.jpg
├── Clouds.jpg
├── .rsvtm
└── .tags
    └── Tracked
        └── Clouds.jpg -> ../../Clouds.jpg
```
