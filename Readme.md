# Readme

pacserver is a local mirror server for pacman running distributions to server multiple computer from a single local download source.

## version
  * version 0.8 

## feature
  * always fetch *db files

## todo
  * threaded (variable) server
  * mirror configuration
  * range accepting
  * logging
  * time management
  * delayed download
  * memory
  * administration tasks (via http)
    * show packages  
    * cleanup refresh
    * prepemtive update 
  
## configure
  * pacman
    * only one downloadthread (disable ParallelDownloads in pacman.conf)
    * use --disable-download-timeout
    * mirrorlist with only server name
    * set paccache to /tmp (or tmpfs filesystem)
 