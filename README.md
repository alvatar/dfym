DFYM
====
(Don't Forget Your Music) is a program that allows you to tag, search and discover files in your computer.

It was born with one purpose: to never forget the music you have. However, it can be used for any collection of files, such as movies, pictures or any other kind of file. Using DFYM, you can search for tagged files (for example, "classical music" or "music for working"), discover untagged files in a directory, tag them, and manage your tags.

Examples:
---------
_Tag a file or directory:_

dfym tag "classical music" "Dvorak - Symphonies No.1-9 - Rafael KubelIk" 

_Search for 3 random files or directories tagged with "work":_

dfym search -rn1 work

_Search for one random directory that hasn't been tagged in a path:_

dfym discover -rdn1 /data/music


More options
------------
_Commands:_

`
tag [tag] [file]          add tag to file or directory
untag [tag] [file]        remove tag from file or directory
show [file]               show the tags of a file directory
tags                      show all defined tags
tagged                    show tagged files
search [tag]              search for files or directories that match this tag
                            flags:
                            -f show only files
                            -d show only directories
                            -nX show only the first X occurences of the query
                            -r randomize order of results
discover [directory]      list untagged files within a given directory
                            flags:
                              -f show only files
                              -d show only directories
                              -nX show only the first X occurences of the query
                              -r randomize order of results
rename [file] [file]      rename files or directories
rename-tag [tag] [tag]    rename a tag
delete [file] [file]      delete files or directories
delete-tag [tag] [tag]    delete a tag
`

