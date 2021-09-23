# fmod_wanderingmind

## How to use
Download fmodapi from [drive](https://drive.google.com/file/d/1x6vUOR-Wd_HBsjHQo_IHatQkx6xeQoOR/view?usp=sharing) or from fmod [site](://fmod.com/download)
and unzip in folder:
`tar -xf fmodstudioapi20201linux.tar.gz`

Have a csv file of audio chunks info with their file path, chunk id, and spatial coordinates (or embeddings).

Change path to mp3_trim in Makefile. mp3_trim should contain trimmed audio files for the aporee dataset.

Build with:
`cmake ./build/`

Run with:
`cd build && ./TEST`

when done clean with
`make clean`



What we do:
- Have the mp3 files from aporee ready and create a simlink in the working directory when building the project.
- We parse the csv file and load them into our data structure.
- We then use FMOD api to load sounds in an compressed format which saves RAM and allows for loading more sounds at once.
- The audio are played according to the position of the dreamer which is fed to FMOD main loop.
- We load and unload audio cleverly and only keep a certain number of audio files loaded at once with the posibility of reusing them if different chunks that are currently playing come from the same file.

TODO:
- Setup an sio client or server to recieve dreamer positions from a remote user.
