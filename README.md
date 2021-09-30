# Repository Organization

This repository contains two branches:
- The main, which contains the original `NET-SD` implmentation.
- The `NCF-NET-SD` implementation, which is removes common blocks.

# NET-SD
This tool is an adaptation of `MRSH-NET`, a similarity search tool based on `MRSH-V2`. This implementation changes the feature extraction tool, which is originally `MRSH-V2`'s to `sdhash`'s, aiming to improve its perfomance.

## Instructions

### Dependencies
- Boost
- OpenSSL (for sha1 hash)
- CMake

#### Boost
1. Extract the library folder onto `NET-SD`'s main folder
 
2. Inside boost folder Run:
```
$ ./bootstrap.sh --prefix = /usr/local/  
$ ./b2 install
```
3. The setup is complete!


### Compiling:
1.  Run:
```  
$ cmake .
$ make
```
## Additional configs
### To modify the threshold of consecutive features  
File ./header/config.h  
```
#define MIN_RUN <number>
```
### To change the Bloom Filter size
File ./header/config.h  
```
#define BF_SIZE_IN_BYTES <number>
```
Obs.: The size must be power of two and expressed in bytes.

### Options 
There are two main options:
- Create BF 
- Search the given bf for files

And there are two ways of making such operations:
- With a list 
- With a dir/file name

- Creation
 - Create a bf with a dir/filename:
 ```
 ./net_sd -g DIR/FILE > filter.sdbf
 ```

 - Create a bf with a list:
 ```
 ./net_sd -d LIST.TXT > filter.sdbf
 ```
- Search
 - search a file/dir into a given bf digest:
 ```
 ./net_sd -c filter.sdbf DIR/FILE > resultados.txt
 ```
 - search a list of files into a given bf digest:
 ```
 ./net_sd -i filter.sdbf list.txt > resultados.txt
 ```

Obs.: I have not tested using a dir inside the given dir, so if you are going to use it please check for possible errors.


# Related Publications
