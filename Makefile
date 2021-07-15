path_to_mp3trim = "/mnt/ACA21355A21322FE/WanderingMind/aporee/mp3_trim"

build:
	mkdir build
	cmake . -B build
	cd build && make
	ln -s ${path_to_mp3trim} mp3

	#mkdir mp3
	#sshfs vthevenin@10.8.0.11:aporee/mp3_trim mp3
	
clean:
	rm -rf build
	rm mp3

	#sudo umount -f mp3
	
run:
	cd build && ./TEST
