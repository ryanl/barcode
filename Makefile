all:
	g++ barcode_recognition.cpp image_handling.cpp ean13.cpp -o barcode_recognition -lgd -lpng -lz -ljpeg -lfreetype -lm -Wall -O3 -march=native

	
