# Custom ls

This program is similar to the Linux ls command which displays directory content and some other optional information about a given directory or a file.

## Building

Run the following commands:

```bash
cmake CMakeLists.txt
make
```

## Usage

```bash
./custom-ls [options] [file_list]
```

### Options

-i                         print the inode number of each file,

-l                         use a long listing format,

-R                         list subdirectories recursively.


## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
See the License.md file
