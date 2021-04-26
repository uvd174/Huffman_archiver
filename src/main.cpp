#include "huffman.h"

#include <iostream>
#include <fstream>
#include <cstring>

int main(int argc, char** argv) {

  try {
    if (argc != 6) {
      throw std::runtime_error("Wrong number of arguments!");
    }
    enum archiver_modes {ENCODE, DECODE};
    std::string in_file, out_file;
    int mode = -3;
    int argi = 1;

    for (int i = 0; i < 3; ++i) {
      if (!strcmp(argv[argi], "-c")) {
        mode = archiver_modes::ENCODE;
        ++argi;
        continue;
      }
      if (!strcmp(argv[argi], "-u")) {
        mode = archiver_modes::DECODE;
        ++argi;
        continue;
      }
      if (!strcmp(argv[argi], "-f") && argi < 5) {
        in_file = argv[++argi];
        ++argi;
        continue;
      }
      if (!strcmp(argv[argi], "-o") && argi < 5) {
        out_file = argv[++argi];
        ++argi;
      }
    }
    if (mode == -3 || in_file.empty() || out_file.empty()) {
      throw std::runtime_error("Wrong arguments!");
    }

    std::ifstream fin(in_file, std::ios::binary);
    if (fin.fail()) {
      throw std::runtime_error("Can't open the input file!");
    }

    std::ofstream fout(out_file, std::ios::binary);
    if (fout.fail()) {
      throw std::runtime_error("Can't open the output file!");
    }

    huff::HuffmanArchiver huffman_archiver;

    long additional_info_size;

    if (mode == archiver_modes::ENCODE) {
      additional_info_size = huffman_archiver.encode(fin, fout);
    } else {
      additional_info_size = huffman_archiver.decode(fin, fout);
    }

    long in_file_size = fin.tellg();
    long out_file_size = fout.tellp();

    if (mode == archiver_modes::ENCODE) {
      out_file_size -= additional_info_size;
    } else {
      in_file_size -= additional_info_size;
    }

    std::cout << in_file_size         << std::endl
              << out_file_size        << std::endl
              << additional_info_size << std::endl;

  } catch (const std::runtime_error &e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}